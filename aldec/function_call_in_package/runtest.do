alib work
set worklib work
alog -dbg example_pkg.sv example_module.sv
asim  +access +w -O2 -dbg example_module
run -all
