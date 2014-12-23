alib work
set worklib work
alog my_interface.sv my_example.sv testcase.sv
asim +access +r -dbg testcase
run -all

