#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <vhpi_user.h>

#define gpi_container_of(_address, _type, _member)  \
        ((_type *)((uintptr_t)(_address) -      \
         (uintptr_t)(&((_type *)0)->_member)))

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


typedef struct gpi_sim_hdl_s {
    void *sim_hdl;
} gpi_sim_hdl_t, *gpi_sim_hdl;

typedef struct t_vhpi_cb_user_data {
    void                *cb_data;
    int                 (*cb_function)(void *);
    int                 (*cleanup_function)(struct t_vhpi_cb_user_data *);
    vhpiHandleT         cb_hdl;
    vhpiValueT          cb_value;
    gpi_sim_hdl_t       gpi_hdl;
    vhpi_cb_state_t     state;
} s_vhpi_cb_user_data, *p_vhpi_cb_user_data;


static gpi_sim_hdl sim_init_cb;
static gpi_sim_hdl sim_finish_cb;
static gpi_sim_hdl root;
static gpi_sim_hdl clk;


static const char * vhpi_reason_to_string(int reason)
{
    switch (reason) {
    case vhpiCbValueChange:
        return "vhpiCbValueChange";
    case vhpiCbStartOfNextCycle:
        return "vhpiCbStartOfNextCycle";
    case vhpiCbStartOfPostponed:
        return "vhpiCbStartOfPostponed";
    case vhpiCbEndOfTimeStep:
        return "vhpiCbEndOfTimeStep";
    case vhpiCbNextTimeStep:
        return "vhpiCbNextTimeStep";
    case vhpiCbAfterDelay:
        return "vhpiCbAfterDelay";
    case vhpiCbStartOfSimulation:
        return "vhpiCbStartOfSimulation";
    case vhpiCbEndOfSimulation:
        return "vhpiCbEndOfSimulation";
    case vhpiCbEndOfProcesses:
        return "vhpiCbEndOfProcesses";
    case vhpiCbLastKnownDeltaCycle:
        return "vhpiCbLastKnownDeltaCycle";
    default:
        return "unknown";
    }
}

static const char * vhpi_format_to_string(int reason)
{
    switch (reason) {
    case vhpiBinStrVal:
        return "vhpiBinStrVal";
    case vhpiOctStrVal:
        return "vhpiOctStrVal";
    case vhpiDecStrVal:
        return "vhpiDecStrVal";
    case vhpiHexStrVal:
        return "vhpiHexStrVal";
    case vhpiEnumVal:
        return "vhpiEnumVal";
    case vhpiIntVal:
        return "vhpiIntVal";
    case vhpiLogicVal:
        return "vhpiLogicVal";
    case vhpiRealVal:
        return "vhpiRealVal";
    case vhpiStrVal:
        return "vhpiStrVal";
    case vhpiCharVal:
        return "vhpiCharVal";
    case vhpiTimeVal:
        return "vhpiTimeVal";
    case vhpiPhysVal:
        return "vhpiPhysVal";
    case vhpiObjTypeVal:
        return "vhpiObjTypeVal";
    case vhpiPtrVal:
        return "vhpiPtrVal";
    case vhpiEnumVecVal:
        return "vhpiEnumVecVal";

    default:
        return "unknown";
    }
}



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


static inline int __gpi_register_cb(p_vhpi_cb_user_data user, vhpiCbDataT *cb_data)
{
    /* If the user data already has a callback handle then deregister
     * before getting the new one
     */
    vhpiHandleT new_hdl = vhpi_register_cb(cb_data, vhpiReturnCb);
    int ret = 0;

    if (!new_hdl) {
        printf("VHPI: Unable to register callback a handle for VHPI type %s(%d)",
                     vhpi_reason_to_string(cb_data->reason), cb_data->reason);
        check_vhpi_error();
        ret = -1;
    }

    if (user->cb_hdl != NULL) {
        printf("VHPI: Attempt to register a callback that's already registered...\n");
        gpi_deregister_callback(&user->gpi_hdl);
    }

    user->cb_hdl = new_hdl;

//     printf("VHPI: Registered %s callback %p\n", vhpi_reason_to_string(cb_data->reason), new_hdl);

    vhpiStateT cbState = vhpi_get(vhpiStateP, new_hdl);
    if (cbState != vhpiEnable) {
        printf("VHPI ERROR: Registered callback isn't enabled! Got %d\n", cbState);
    }

    return ret;
}

static inline p_vhpi_cb_user_data __gpi_alloc_user(void)
{
    printf("calloc for user_data\n");
    p_vhpi_cb_user_data new_data = calloc(1, sizeof(*new_data));
    if (new_data == NULL) {
        printf("VHPI: Attempting allocate user_data failed!");
    }

    return new_data;
}

static inline void __gpi_free_callback(gpi_sim_hdl gpi_hdl)
{
    p_vhpi_cb_user_data user_data;
    user_data = gpi_container_of(gpi_hdl, s_vhpi_cb_user_data, gpi_hdl);

    free(user_data);
}

/* Allocates memory that will persist for the lifetime of the
 * handle, this may be short or long. A call to create
 * must have a matching call to destroy at some point
 */
gpi_sim_hdl gpi_create_cb_handle(void)
{
    gpi_sim_hdl ret = NULL;

    p_vhpi_cb_user_data user_data = __gpi_alloc_user();
    if (user_data) {
        user_data->state = VHPI_FREE;
        ret = &user_data->gpi_hdl;
    }

    return ret;
}

void gpi_free_handle(gpi_sim_hdl gpi_hdl)
{
    free(gpi_hdl);
}

static gpi_sim_hdl gpi_alloc_handle(void)
{
    printf("alloc for sim_hdl\n");
    gpi_sim_hdl new_hdl = calloc(1, sizeof(*new_hdl));
    if (!new_hdl) {
        printf("VPI: Could not allocate handle\n");
        exit(1);
    }

    return new_hdl;
}

/* Destroys the memory associated with the sim handle
 * this can only be called on a handle that has been
 * returned by a call to gpi_create_cb_handle
 */
void gpi_destroy_cb_handle(gpi_sim_hdl gpi_hdl)
{
    /* Check that is has been called, if this has not
     * happend then also close down the sim data as well
     */
    p_vhpi_cb_user_data user_data;
    user_data = gpi_container_of(gpi_hdl, s_vhpi_cb_user_data, gpi_hdl);

    if (user_data->state == VHPI_PRE_CALL) {
        user_data->state = VHPI_DELETE;
    } else {
        gpi_deregister_callback(gpi_hdl);
        __gpi_free_callback(gpi_hdl);
    }
}


/* Deregister a prior set up callback with the simulator
 * The handle must have been allocated with gpi_create_cb_handle
 * This can be called at any point between
 * gpi_create_cb_handle and gpi_destroy_cb_handle
 */
int gpi_deregister_callback(gpi_sim_hdl gpi_hdl)
{
    p_vhpi_cb_user_data user_data;
    int rc = 1;

    user_data = gpi_container_of(gpi_hdl, s_vhpi_cb_user_data, gpi_hdl);

    if (user_data->cb_hdl != NULL) {
//         printf("GPI deregister callback on %p\n", user_data->cb_hdl);
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
static int gpi_free_one_time(p_vhpi_cb_user_data user_data)
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

// Call when the handle relates to recurring callback
// Unregister must be called when not needed and this
// will clean all memory allocated by the sim
static int gpi_free_recurring(p_vhpi_cb_user_data user_data)
{
    int rc;
    vhpiHandleT cb_hdl = user_data->cb_hdl;
    if (!cb_hdl) {
        printf("VPI: passed a NULL pointer : ABORTING\n");
        exit(1);
    }

    rc = vhpi_remove_cb(cb_hdl);
    check_vhpi_error();
    profile.vhpi_remove_count++;
    return rc;
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
    rv = user_data->cb_function(user_data->cb_data);


    /* A request to delete could have been done
     * inside gpi_function
     */
    if (user_data->state == VHPI_DELETE)
        gpi_destroy_cb_handle(&user_data->gpi_hdl);
    else
        user_data->state = VHPI_POST_CALL;

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



// Handle related functions
/**
 * @name    Find the root handle
 * @brief   Find the root handle using a optional name
 *
 * Get a handle to the root simulator object.  This is usually the toplevel.
 *
 * FIXME: In VHPI we always return the first root instance
 * 
 * TODO: Investigate possibility of iterating and checking names as per VHPI
 * If no name is defined, we return the first root instance.
 *
 * If name is provided, we check the name against the available objects until
 * we find a match.  If no match is found we return NULL
 */
gpi_sim_hdl gpi_get_root_handle(const char* name)
{
    vhpiHandleT root;
    vhpiHandleT dut;
    gpi_sim_hdl rv;

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

    rv = gpi_alloc_handle();
    rv->sim_hdl = dut;

    return rv;
}

gpi_sim_hdl gpi_get_handle_by_name(const char *name, gpi_sim_hdl parent)
{
    gpi_sim_hdl rv;
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
    obj = vhpi_handle_by_name(buff, (vhpiHandleT)(parent->sim_hdl));
    if (!obj) {
        printf("VHPI: Handle '%s' not found!\n", name);
        check_vhpi_error();
        return NULL;
    }

    free(buff);

    rv = gpi_alloc_handle();
    rv->sim_hdl = obj;

    return rv;
}




int gpi_register_sim_phase_cb(      uint32_t reason,
                                    gpi_sim_hdl cb,
                                    int (*cb_function)(void *),
                                    void *cb_data)
{
    vhpiCbDataT cb_data_s;
    p_vhpi_cb_user_data user_data;

    user_data = gpi_container_of(cb, s_vhpi_cb_user_data, gpi_hdl);

    user_data->cb_data = cb_data;
    user_data->cb_function = cb_function;
    user_data->cleanup_function = gpi_free_one_time;

    cb_data_s.reason    = reason;
    cb_data_s.cb_rtn    = handle_vhpi_callback;
    cb_data_s.obj       = NULL;
    cb_data_s.time      = NULL;
    cb_data_s.value     = NULL;
    cb_data_s.user_data = (char *)user_data;

    __gpi_register_cb(user_data, &cb_data_s);
    user_data->state = VHPI_PRIMED;
    return 0;
}

int gpi_register_timed_callback(gpi_sim_hdl cb,
                                int (*gpi_function)(void *),
                                void *gpi_cb_data,
                                uint64_t time_ps)
{
    vhpiCbDataT cb_data_s;
    vhpiTimeT time_s;

    p_vhpi_cb_user_data user_data;
    int ret;

    user_data = gpi_container_of(cb, s_vhpi_cb_user_data, gpi_hdl);

    user_data->cb_data = gpi_cb_data;
    user_data->cb_function = gpi_function;
    user_data->cleanup_function = gpi_free_one_time;

    time_s.high = (uint32_t)(time_ps>>32);
    time_s.low  = (uint32_t)(time_ps);

    cb_data_s.reason    = vhpiCbAfterDelay;
    cb_data_s.cb_rtn    = handle_vhpi_callback;
    cb_data_s.obj       = NULL;
    cb_data_s.time      = &time_s;
    cb_data_s.value     = NULL;
    cb_data_s.user_data = (char *)user_data;

    ret = __gpi_register_cb(user_data, &cb_data_s);
    user_data->state = VHPI_PRIMED;
    return ret;
}

// Unfortunately it seems that format conversion is not well supported
// We have to set values using vhpiEnum*
void gpi_set_signal_value_int(gpi_sim_hdl gpi_hdl, int value)
{
    vhpiValueT value_s;
    vhpiValueT *value_p = &value_s;
    int size, i;

    // Determine the type of object, either scalar or vector
    value_s.format = vhpiObjTypeVal;
    value_s.bufSize = 0;
    value_s.value.str = NULL;

    vhpi_get_value((vhpiHandleT)(gpi_hdl->sim_hdl), &value_s);
    check_vhpi_error();

    switch (value_s.format) {
        case vhpiEnumVal: {
            value_s.value.enumv = value ? vhpi1 : vhpi0;
            break;
        }

        case vhpiEnumVecVal: {
            size = vhpi_get(vhpiSizeP, (vhpiHandleT)(gpi_hdl->sim_hdl));
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

    vhpi_put_value((vhpiHandleT)(gpi_hdl->sim_hdl), &value_s, vhpiForcePropagate);
    check_vhpi_error();

    if (vhpiEnumVecVal == value_s.format)
        free(value_s.value.enumvs);
}


typedef struct gpi_clock_s {
    int period;
    int value;
    unsigned int max_cycles;
    unsigned int curr_cycle;
    gpi_sim_hdl_t gpi_hdl;  /* Handle to pass back to called */
    gpi_sim_hdl clk_hdl;    /* Handle for signal to operate on */
    gpi_sim_hdl cb_hdl;     /* Handle for the current pending callback */
} gpi_clock_t;
typedef gpi_clock_t *gpi_clock_hdl;


// Actual example code... firstly we create a clock
int gpi_clock_handler(void *clock)
{
    gpi_clock_hdl hdl = (gpi_clock_hdl)clock;
    gpi_sim_hdl cb_hdl;

    if ((hdl->max_cycles != 0) && (hdl->max_cycles == hdl->curr_cycle))
        return;

    /* Unregister/free the last callback that just fired */
    cb_hdl = hdl->cb_hdl;
    gpi_deregister_callback(cb_hdl);

    hdl->value = !hdl->value;
    gpi_set_signal_value_int(hdl->clk_hdl, hdl->value);

    gpi_register_timed_callback(cb_hdl, gpi_clock_handler, hdl, hdl->period);
    hdl->curr_cycle++;
}

gpi_sim_hdl gpi_create_clock(gpi_sim_hdl sim_hdl, int period, unsigned int cycles)
{
    printf("Malloc for gpi_clock_t\n");
    gpi_clock_hdl hdl = malloc(sizeof(gpi_clock_t));


    if (!hdl) {
        printf("VPI: Unable to allocate memory\n");
        exit(1);
    }

    hdl->period = period;
    hdl->value = 0;
    hdl->clk_hdl = sim_hdl;
    hdl->max_cycles = cycles;
    hdl->curr_cycle = 0;

    gpi_set_signal_value_int(hdl->clk_hdl, hdl->value);
    hdl->cb_hdl = gpi_create_cb_handle();

    gpi_register_timed_callback(hdl->cb_hdl, gpi_clock_handler, hdl, hdl->period);

    return &hdl->gpi_hdl;
}











int handle_sim_init(void *cb_data)
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

int handle_sim_end(void *cb_data)
{
    printf("Simulation finished\n");
}

void register_initial_callback(void)
{
    sim_init_cb = gpi_create_cb_handle();
    gpi_register_sim_phase_cb(vhpiCbStartOfSimulation, sim_init_cb, handle_sim_init, (void *)NULL);
}


void register_final_callback(void)
{
    sim_finish_cb = gpi_create_cb_handle();
    gpi_register_sim_phase_cb(vhpiCbEndOfSimulation, sim_finish_cb, handle_sim_end, (void *)NULL);
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
