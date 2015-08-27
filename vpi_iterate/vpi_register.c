#include <stdio.h>
#include "vpi_user.h"


static void check_vpi_error(void)
{
    s_vpi_error_info info;
    int level;

    memset(&info, 0, sizeof(info));
    level = vpi_chk_error(&info);
    if (info.code == 0 && level == 0)
        return;

    printf("VPI Error %s\nPROD %s\nCODE %s\nFILE %s",
           info.message, info.product, info.code, info.file);
    return;
}

vpiHandle get_root_module(void) 
{
    vpiHandle iterator;
    vpiHandle root;

    iterator = vpi_iterate(vpiModule, NULL);
    check_vpi_error();

    root = vpi_scan(iterator);

    if (NULL != root) {
        // Need to free the iterator if it didn't return NULL
        vpi_free_object(iterator);
        check_vpi_error();
    }
    return root;
}


void do_iterate(vpiHandle handle, PLI_INT32 type)
{
    vpiHandle iterator;
    vpiHandle thing;
    iterator = vpi_iterate(type, handle);
    if (NULL == iterator) {
        printf("No objects found for type %d\n", type);
        return;
    }
    printf("Scanning %d...\n", type);
    for (thing = vpi_scan(iterator); thing != NULL; thing = vpi_scan(iterator)) {
        printf("    %d: Found %s\n", type, vpi_get_str(vpiFullName, thing));
    }
    return;
}




PLI_INT32 elaboration_callback(p_cb_data cb_data_p)
{
    printf("In elaboration callback\n");
    vpiHandle root = get_root_module();

    if (NULL == root) {
        printf("Failed to get root handle\n");
        return 0;
    }
    printf("Found root handle %s\n", vpi_get_str(vpiFullName, root));

    // Valid mappings according to VPI spec
    do_iterate(root, vpiModule);
    do_iterate(root, vpiModuleArray);
    do_iterate(root, vpiInternalScope);
    do_iterate(root, vpiPort);
    do_iterate(root, vpiIODecl);
    do_iterate(root, vpiNet);
    do_iterate(root, vpiNetArray);
    do_iterate(root, vpiReg);
    do_iterate(root, vpiRegArray);
    do_iterate(root, vpiMemory);
    do_iterate(root, vpiVariables);
    do_iterate(root, vpiNamedEvent);
    do_iterate(root, vpiNamedEventArray);
    do_iterate(root, vpiParameter);
    do_iterate(root, vpiSpecParam);
    do_iterate(root, vpiParamAssign);
    do_iterate(root, vpiDefParam);
    do_iterate(root, vpiPrimitive);
    do_iterate(root, vpiPrimitiveArray);
    do_iterate(root, vpiContAssign);
    do_iterate(root, vpiProcess);
    do_iterate(root, vpiModPath);
    do_iterate(root, vpiTchk);
}

PLI_INT32 register_elaboration_callback(void)
{
    s_cb_data callback;
    callback.reason = cbStartOfSimulation;
    callback.cb_rtn = elaboration_callback;
    callback.user_data = 0;
    vpi_register_cb(&callback);
    return 0;
}

void (*vlog_startup_routines[]) () = {
    register_elaboration_callback,
    0
};
