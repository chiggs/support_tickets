#include <stdio.h>
#include "vpi_user.h"
#include "sv_vpi_user.h"

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
        case vpiStructVar        : return "vpiStructVar";
        case vpiIntVar           : return "vpiIntVar";
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



vpiHandle by_name(const char *name, vpiHandle scope)
{
    vpiHandle result = vpi_handle_by_name(name, scope);
    check_vpi_error();
    int32_t type = vpi_get(vpiType, result);
    check_vpi_error();
    printf("Found %s of type %s (%d) by name\n", vpi_get_str(vpiFullName, result), to_str(type), type);
    return result;
}

vpiHandle by_index(int index, vpiHandle scope)
{
    vpiHandle result = vpi_handle_by_index(scope, index);
    check_vpi_error();
    int32_t type = vpi_get(vpiType, result);
    check_vpi_error();
    printf("Found %s of type %s (%d) by index\n", vpi_get_str(vpiFullName, result), to_str(type), type);
    return result;
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

    vpiHandle nconfig_0 = by_name("normal_config.uint32_signal",        root);
    vpiHandle nconfig_1 = by_name("normal_config.uint64_array",         root);
    vpiHandle nconfig_2 = by_name("normal_config.uint64_signal",        root);
    vpiHandle nconfig_3 = by_name("normal_config.logic_array",          root);
    vpiHandle nconfig_4 = by_name("normal_config.uint32_signal_local",  root);
    vpiHandle nconfig_5 = by_name("normal_config.uint64_signal_local",  root);
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
