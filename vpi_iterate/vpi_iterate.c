#include <stdio.h>
#include "vpi_user.h"

static const char *to_str(PLI_INT32 type)
{
    switch (type) {
        case vpiAlways           : return "vpiAlways";
        case vpiAssignStmt       : return "vpiAssignStmt";
        case vpiAssignment       : return "vpiAssignment";
        case vpiBegin            : return "vpiBegin";
        case vpiCase             : return "vpiCase";
        case vpiCaseItem         : return "vpiCaseItem";
        case vpiConstant         : return "vpiConstant";
        case vpiContAssign       : return "vpiContAssign";
        case vpiDeassign         : return "vpiDeassign";
        case vpiDefParam         : return "vpiDefParam";
        case vpiDelayControl     : return "vpiDelayControl";
        case vpiDisable          : return "vpiDisable";
        case vpiEventControl     : return "vpiEventControl";
        case vpiEventStmt        : return "vpiEventStmt";
        case vpiFor              : return "vpiFor";
        case vpiForce            : return "vpiForce";
        case vpiForever          : return "vpiForever";
        case vpiFork             : return "vpiFork";
        case vpiFuncCall         : return "vpiFuncCall";
        case vpiFunction         : return "vpiFunction";
        case vpiGate             : return "vpiGate";
        case vpiIf               : return "vpiIf";
        case vpiIfElse           : return "vpiIfElse";
        case vpiInitial          : return "vpiInitial";
        case vpiIntegerVar       : return "vpiIntegerVar";
        case vpiInterModPath     : return "vpiInterModPath";
        case vpiIterator         : return "vpiIterator";
        case vpiIODecl           : return "vpiIODecl";
        case vpiMemory           : return "vpiMemory";
        case vpiMemoryWord       : return "vpiMemoryWord";
        case vpiModPath          : return "vpiModPath";
        case vpiModport          : return "vpiModport";
        case vpiModule           : return "vpiModule";
        case vpiNamedBegin       : return "vpiNamedBegin";
        case vpiNamedEvent       : return "vpiNamedEvent";
        case vpiNamedFork        : return "vpiNamedFork";
        case vpiNet              : return "vpiNet";
        case vpiNetBit           : return "vpiNetBit";
        case vpiNullStmt         : return "vpiNullStmt";
        case vpiOperation        : return "vpiOperation";
        case vpiParamAssign      : return "vpiParamAssign";
        case vpiParameter        : return "vpiParameter";
        case vpiPartSelect       : return "vpiPartSelect";
        case vpiPathTerm         : return "vpiPathTerm";
        case vpiPort             : return "vpiPort";
        case vpiPortBit          : return "vpiPortBit";
        case vpiPrimTerm         : return "vpiPrimTerm";
        case vpiRealVar          : return "vpiRealVar";
        case vpiReg              : return "vpiReg";
        case vpiRegBit           : return "vpiRegBit";
        case vpiRelease          : return "vpiRelease";
        case vpiRepeat           : return "vpiRepeat";
        case vpiRepeatControl    : return "vpiRepeatControl";
        case vpiSchedEvent       : return "vpiSchedEvent";
        case vpiSpecParam        : return "vpiSpecParam";
        case vpiSwitch           : return "vpiSwitch";
        case vpiSysFuncCall      : return "vpiSysFuncCall";
        case vpiSysTaskCall      : return "vpiSysTaskCall";
        case vpiTableEntry       : return "vpiTableEntry";
        case vpiTask             : return "vpiTask";
        case vpiTaskCall         : return "vpiTaskCall";
        case vpiTchk             : return "vpiTchk";
        case vpiTchkTerm         : return "vpiTchkTerm";
        case vpiTimeVar          : return "vpiTimeVar";
        case vpiTimeQueue        : return "vpiTimeQueue";
        case vpiUdp              : return "vpiUdp";
        case vpiUdpDefn          : return "vpiUdpDefn";
        case vpiUserSystf        : return "vpiUserSystf";
        case vpiVarSelect        : return "vpiVarSelect";
        case vpiWait             : return "vpiWait";
        case vpiWhile            : return "vpiWhile";
        case vpiAttribute        : return "vpiAttribute";
        case vpiBitSelect        : return "vpiBitSelect";
        case vpiCallback         : return "vpiCallback";
        case vpiDelayTerm        : return "vpiDelayTerm";
        case vpiDelayDevice      : return "vpiDelayDevice";
        case vpiFrame            : return "vpiFrame";
        case vpiGateArray        : return "vpiGateArray";
        case vpiModuleArray      : return "vpiModuleArray";
        case vpiPrimitiveArray   : return "vpiPrimitiveArray";
        case vpiNetArray         : return "vpiNetArray";
        case vpiRange            : return "vpiRange";
        case vpiRegArray         : return "vpiRegArray";
        case vpiSwitchArray      : return "vpiSwitchArray";
        case vpiUdpArray         : return "vpiUdpArray";
        case vpiContAssignBit    : return "vpiContAssignBit";
        case vpiNamedEventArray  : return "vpiNamedEventArray";
        case vpiIndexedPartSelect: return "vpiIndexedPartSelect";
        case vpiGenScopeArray    : return "vpiGenScopeArray";
        case vpiGenScope         : return "vpiGenScope";
        case vpiGenVar           : return "vpiGenVar";
        case vpiVariables        : return "vpiVariables";
        default                  : return "UNKNOWN"; 
    }
}

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
    printf("Attempting to iterate over type %s (%d)\n", to_str(type), type);
    iterator = vpi_iterate(type, handle);
    if (NULL == iterator) {
        printf("No objects found for type %s (%d)\n", to_str(type), type);
        return;
    }
    printf("Scanning %s (%d)...\n", to_str(type), type);
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
