
#include <stddef.h>
#include <vpi_user.h>

static void print_err(void)
{
    s_vpi_error_info info;
    vpi_chk_error(&info);
    vpi_printf("ERROR: %s (%d): %s\n", info.file, info.code, info.message);
}


static vpiHandle get_subhandle(char *name, vpiHandle parent)
{
    vpi_printf("Attempting to get subhandle \"%s\" from %s\n", name, vpi_get_str(vpiFullName, parent));

    vpiHandle rv = vpi_handle_by_name(name, parent);
    if (!rv) {
        print_err();
        vpi_printf("FAILED: Unable to get subhandle %s.%s\n", vpi_get_str(vpiFullName, parent), name);
        vpi_control(vpiFinish, 0);
        return NULL;
    }
    return rv;
}


static int32_t get_handles(p_cb_data cb_data)
{
    // Find the root handle
    vpiHandle root;
    vpiHandle iterator;

    iterator = vpi_iterate(vpiModule, NULL);
    root = vpi_scan(iterator);

    if (!root) {
        print_err();
        vpi_printf("FAILED: Unable to get root handle\n");
        vpi_control(vpiFinish, 0);
        return -1;
    } else if (!vpi_free_object(iterator)) {
        vpi_printf("Unable to free iterator\n");
    }

    vpi_printf("Found root: %s\n", vpi_get_str(vpiFullName, root));

    // Try and get handles to structure members by name
    vpiHandle struct_output = get_subhandle("struct_output", root);
    if (!struct_output)
        return -1;
  
    vpiHandle struct_valid = get_subhandle("valid", struct_output);
    if (!struct_valid)
        return -1;

    vpiHandle a_substruct = get_subhandle("a_substruct", struct_output);
    if (!a_substruct)
        return -1;

    vpiHandle a_flag = get_subhandle("a_flag", a_substruct);
    if (!a_flag)
        return -1;

    vpiHandle a_vector = get_subhandle("a_vector", a_substruct);
    if (!a_vector)
        return -1;
 
    return 0;
}



static void register_initial_callback(void)
{
    s_cb_data cb_data_s;

    cb_data_s.reason    = cbStartOfSimulation;
    cb_data_s.cb_rtn    = get_handles;
    cb_data_s.obj       = NULL;
    cb_data_s.time      = NULL;
    cb_data_s.value     = NULL;
    cb_data_s.user_data = NULL;
    
    vpiHandle new_hdl = vpi_register_cb(&cb_data_s);
    if (!new_hdl) {
        vpi_printf("**** ERROR: Could not register the callback\n");
        vpi_control(vpiFinish, 0);
    }
}



void (*vlog_startup_routines[])(void) = {
    register_initial_callback,
    0
};


