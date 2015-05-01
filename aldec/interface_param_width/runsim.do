alib interface_lib
set worklib interface_lib
alog -dbg my_interface.sv

alib testcase_lib
set worklib testcase_lib
alog -l interface_lib -dbg my_module.sv my_testbench.sv
asim -L interface_lib testcase_lib.testcase
run -all
quit
