#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <vhpi_user.h>

#define VHPI_CHECKING 1

typedef struct {
    uint64_t            register_count;
    uint64_t            vhpi_remove_count;
    uint64_t            vhpi_release_count;
    uint64_t            callback_count;
    uint64_t            nano_seconds_in_sim;
    uint64_t            nano_seconds_in_gpi;
    uint64_t            last_callback;
} profile_t;

static profile_t    profile;


uint64_t to_nanos(struct timespec *t)
{
    uint64_t rv = t->tv_nsec;
    rv += (t->tv_sec * 10e9);
    return rv;
}

typedef enum vhpi_cb_state_e {
    VHPI_FREE = 0,
    VHPI_PRIMED = 1,
    VHPI_PRE_CALL = 2,
    VHPI_POST_CALL = 3,
    VHPI_DELETE = 4,
} vhpi_cb_state_t;

// Define a type of a clock object
typedef struct gpi_clock_s {
    int                 period;
    int                 value;
    unsigned int        max_cycles;
    unsigned int        curr_cycle;
    vhpiHandleT         clk_hdl;
} gpi_clock_t , *p_clock_state;


// This is the user data we pass in with the callback
typedef struct t_vhpi_cb_user_data {
    void               *my_data;
    int                 (*cb_function)(struct t_vhpi_cb_user_data *);
    int                 (*cleanup_function)(struct t_vhpi_cb_user_data *);
    vhpiHandleT         cb_hdl;
    vhpi_cb_state_t     state;
} s_vhpi_cb_user_data, *p_vhpi_cb_user_data;


// Static user data for sim start/end
static s_vhpi_cb_user_data sim_init_cb;
static s_vhpi_cb_user_data sim_finish_cb;

// Handles to signals
static vhpiHandleT  root;
static vhpiHandleT  clk;

// Should be run after every VPI call to check error status
static inline int __check_vhpi_error(const char *func, long line)
{
    int level = 0;
#if VHPI_CHECKING
    vhpiErrorInfoT info;
    level = vhpi_check_error(&info);

    if (level == 0)
        return level;

    printf("Error %s:%d\tVHPI Error level %d: %s FILE %s:%d", 
               func, line, info.severity, info.message, info.file, info.line);
#endif
    return level;
}

#define check_vhpi_error() \
    __check_vhpi_error(__func__, __LINE__)


static int register_cb(vhpiCbDataT *cb_data)
{
    /* If the user data already has a callback handle then deregister
     * before getting the new one
     */
    int ret = 0;
    p_vhpi_cb_user_data user;
    user = cb_data->user_data;

    if (user->cb_hdl) {
        printf("VHPI: Attempt to register a callback that's already registered...\n");
        return -1;
    }

    vhpiHandleT new_hdl = vhpi_register_cb(cb_data, vhpiReturnCb);

    if (!new_hdl) {
        printf("VHPI: Unable to register callback a handle for VHPI type %s(%d)",
                     vhpi_reason_to_string(cb_data->reason), cb_data->reason);
        check_vhpi_error();
        ret = -1;
    }

    user->cb_hdl = new_hdl;

    vhpiStateT cbState = vhpi_get(vhpiStateP, new_hdl);
    if (cbState != vhpiEnable) {
        printf("VHPI ERROR: Registered callback isn't enabled! Got %d\n", cbState);
    }

    return ret;
}


/* Deregister a prior set up callback with the simulator
 * The handle must have been allocated with gpi_create_cb_handle
 * This can be called at any point between
 * gpi_create_cb_handle and gpi_destroy_cb_handle
 */
int gpi_deregister_callback(p_vhpi_cb_user_data user_data)
{
    int rc = 1;

    if (user_data->cb_hdl != NULL) {
        rc = user_data->cleanup_function(user_data);
        user_data->cb_hdl = NULL;
    } else {
        printf("NULL cb_hdl, not cleaning up\n");
    }

    return rc;
}


// Call when the handle relates to a one time callback
// No need to call vhpi_deregister_cb as the sim will
// do this but do need to destroy the handle
static int vhpi_free_one_time(p_vhpi_cb_user_data user_data)
{
    int rc = 0;
    vhpiHandleT cb_hdl = user_data->cb_hdl;
    if (!cb_hdl) {
        printf("VPI: passed a NULL pointer : ABORTING\n");
        exit(1);
    }

    // If the callback has not been called we also need to call
    // remove as well
    if (user_data->state == VHPI_PRIMED) {
        rc = vhpi_remove_cb(cb_hdl);
        if (!rc) {
            check_vhpi_error();
            printf("vhpi_remove_cb failed with %d\n", rc);
            return rc;
        }
        profile.vhpi_remove_count++;
    } else {
        rc = vhpi_release_handle(cb_hdl);
        if (rc) {
            check_vhpi_error();
            printf("vhpi_release_handle failed with %d\n", rc);
            return rc;
        }
    }
    profile.vhpi_release_count++;
    return rc;
}


int gpi_clock_handler(p_vhpi_cb_user_data user_data)
{
    p_clock_state clock_state = (p_clock_state)user_data->my_data;

    /* Unregister/free the last callback that just fired */
    gpi_deregister_callback(user_data);

    if ((clock_state->max_cycles != 0) && (clock_state->max_cycles == clock_state->curr_cycle))
        return;

    clock_state->value = !clock_state->value;
    gpi_set_signal_value_int(clock_state->clk_hdl, clock_state->value);

    gpi_register_timed_callback(user_data, gpi_clock_handler, clock_state->period);
    clock_state->curr_cycle++;
}




static void handle_vhpi_callback(const vhpiCbDataT *cb_data)
{
    struct timespec now;
    int rv = 0;
    vhpiHandleT old_cb;
    clock_gettime(CLOCK_REALTIME, &now);
    uint64_t tstart = to_nanos(&now);

    profile.callback_count++;

    if (profile.last_callback) {
        profile.nano_seconds_in_sim += tstart - profile.last_callback;
    }

    p_vhpi_cb_user_data user_data;
    user_data = (p_vhpi_cb_user_data)cb_data->user_data;

    if (!user_data)
        printf("VPI: Callback data corrupted\n");

    user_data->state = VHPI_PRE_CALL;
    old_cb = user_data->cb_hdl;
    rv = user_data->cb_function(user_data);

    clock_gettime(CLOCK_REALTIME, &now);
    profile.last_callback = to_nanos(&now);

    profile.nano_seconds_in_gpi += profile.last_callback - tstart;

    if (profile.callback_count && !(profile.callback_count % 100000)) {
        printf("Profile after %d callbacks (%d removed)\t(%d released):\n", profile.callback_count, profile.vhpi_remove_count, profile.vhpi_release_count);
        printf("\tTime in sim: %luns\t\t(avg: %f)\n", profile.nano_seconds_in_sim, (double)profile.nano_seconds_in_sim / (double)profile.callback_count);
        printf("\tTime in gpi: %luns\t\t(avg: %f)\n", profile.nano_seconds_in_gpi, (double)profile.nano_seconds_in_gpi / (double)profile.callback_count);
    }
    return;
};


vhpiHandleT gpi_get_root_handle(const char* name)
{
    vhpiHandleT root;
    vhpiHandleT dut;

    root = vhpi_handle(vhpiRootInst, NULL);
    check_vhpi_error();

    if (!root) {
        printf("VHPI: Attempting to get the root handle failed");
        return NULL;
    }

    if (name)
        dut = vhpi_handle_by_name(name, NULL);
    else
        dut = vhpi_handle(vhpiDesignUnit, root);
    check_vhpi_error();

    if (!dut) {
        printf("VHPI: Attempting to get the DUT handle failed");
        return NULL;
    }

    const char *found = vhpi_get_str(vhpiNameP, dut);
    check_vhpi_error();

    if (name != NULL && strcmp(name, found)) {
        printf("VHPI: Root '%s' doesn't match requested toplevel %s", found, name);
        return NULL;
    }
    return dut;
}

vhpiHandleT gpi_get_handle_by_name(const char *name, vhpiHandleT parent)
{
    vhpiHandleT obj;
    int len;
    char *buff;
    if (name)
        len = strlen(name) + 1;

    printf("malloc for get_handle_by_name\n");
    buff = (char *)malloc(len);
    if (buff == NULL) {
        printf("VHPI: Attempting allocate string buffer failed!\n");
        return NULL;
    }

    strncpy(buff, name, len);
    obj = vhpi_handle_by_name(buff, parent);
    if (!obj) {
        printf("VHPI: Handle '%s' not found!\n", name);
        check_vhpi_error();
        return NULL;
    }

    free(buff);
    return obj;
}




int gpi_register_sim_phase_cb(      int32_t reason,
                                    p_vhpi_cb_user_data user_data,
                                    int (*cb_function)(p_vhpi_cb_user_data))
{
    vhpiCbDataT cb_data_s;
    int ret;

    user_data->cb_function = cb_function;
    user_data->cleanup_function = vhpi_free_one_time;

    cb_data_s.reason    = reason;
    cb_data_s.cb_rtn    = handle_vhpi_callback;
    cb_data_s.obj       = NULL;
    cb_data_s.time      = NULL;
    cb_data_s.value     = NULL;
    cb_data_s.user_data = (char *)user_data;

    ret = register_cb(&cb_data_s);
    user_data->state = VHPI_PRIMED;
    return ret;
}

int gpi_register_timed_callback(p_vhpi_cb_user_data user_data,
                                int (*gpi_function)(p_vhpi_cb_user_data),
                                uint64_t time_ps)
{
    vhpiCbDataT cb_data_s;
    vhpiTimeT time_s;
    int ret;

    user_data->cb_function = gpi_function;
    user_data->cleanup_function = vhpi_free_one_time;

    time_s.high = (uint32_t)(time_ps>>32);
    time_s.low  = (uint32_t)(time_ps);

    cb_data_s.reason    = vhpiCbAfterDelay;
    cb_data_s.cb_rtn    = handle_vhpi_callback;
    cb_data_s.obj       = NULL;
    cb_data_s.time      = &time_s;
    cb_data_s.value     = NULL;
    cb_data_s.user_data = (char *)user_data;

    ret = register_cb(&cb_data_s);
    user_data->state = VHPI_PRIMED;
    return ret;
}

// Unfortunately it seems that format conversion is not well supported
// We have to set values using vhpiEnum*
void gpi_set_signal_value_int(vhpiHandleT hdl, int value)
{
    vhpiValueT value_s;
    vhpiValueT *value_p = &value_s;
    int size, i;

    // Determine the type of object, either scalar or vector
    value_s.format = vhpiObjTypeVal;
    value_s.bufSize = 0;
    value_s.value.str = NULL;

    vhpi_get_value(hdl, &value_s);
    check_vhpi_error();

    switch (value_s.format) {
        case vhpiEnumVal: {
            value_s.value.enumv = value ? vhpi1 : vhpi0;
            break;
        }

        case vhpiEnumVecVal: {
            size = vhpi_get(vhpiSizeP, hdl);
            value_s.bufSize = size*sizeof(vhpiEnumT); 
            value_s.value.enumvs = (vhpiEnumT *)malloc(size*sizeof(vhpiEnumT));
            printf("malloc for value_s.value.enumvs\n");

            for (i=0; i<size; i++)
                value_s.value.enumvs[size-i-1] = value&(1<<i) ? vhpi1 : vhpi0;

            break;
        }

        default: {
            printf("Unable to assign value to %s (%d) format object\n",
                         vhpi_format_to_string(value_s.format), value_s.format);
        }
    }

    vhpi_put_value(hdl, &value_s, vhpiForcePropagate);
    check_vhpi_error();

    if (vhpiEnumVecVal == value_s.format)
        free(value_s.value.enumvs);
}




p_vhpi_cb_user_data gpi_create_clock(vhpiHandleT clk_hdl, int period, unsigned int cycles)
{
    p_clock_state state = malloc(sizeof(gpi_clock_t));
    p_vhpi_cb_user_data user_data = malloc(sizeof(s_vhpi_cb_user_data));

    if (!state) {
        printf("VPI: Unable to allocate memory\n");
        exit(1);
    }

    state->period = period;
    state->value = 0;
    state->clk_hdl = clk_hdl;
    state->max_cycles = cycles;
    state->curr_cycle = 0;

    gpi_set_signal_value_int(state->clk_hdl, state->value);

    user_data->my_data = state;
    user_data->cb_hdl = NULL;
    printf("Registering first timed callback....\n");
    gpi_register_timed_callback(user_data, gpi_clock_handler, state->period);
    printf("Done!\n");
    return user_data;
}








int handle_sim_init(p_vhpi_cb_user_data user_data)
{
    profile.callback_count = 0;
    profile.nano_seconds_in_gpi = 0;
    profile.nano_seconds_in_sim = 0;
    profile.last_callback = 0;

    root = gpi_get_root_handle("vhpi_example");
    printf("Found root handle: %p\n", root);

    clk = gpi_get_handle_by_name("clk", root);
    printf("Found clk handle: %p\n", clk);

    gpi_create_clock(clk, 5000, 0);
}

int handle_sim_end(p_vhpi_cb_user_data user_data)
{
    printf("Simulation finished\n");
}

void register_initial_callback(void)
{
    gpi_register_sim_phase_cb(vhpiCbStartOfSimulation, &sim_init_cb, handle_sim_init);
}


void register_final_callback(void)
{
    gpi_register_sim_phase_cb(vhpiCbEndOfSimulation, &sim_finish_cb, handle_sim_end);
}



// pre-defined VHPI registration table
void (*vhpi_startup_routines[])(void) = {
    register_initial_callback,
    register_final_callback,
    0
};

// For non-VPI compliant applications that cannot find vlog_startup_routines
void vhpi_startup_routines_bootstrap(void) {
    void (*routine)(void);
    int i;
    routine = vhpi_startup_routines[0];
    for (i = 0, routine = vhpi_startup_routines[i];
         routine;
         routine = vhpi_startup_routines[++i]) {
        routine();
    }
}
