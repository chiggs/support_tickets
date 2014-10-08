alib work
set worklib work
alog -dbg testcase.sv
asim  -dbg testcase
wave sim:/testcase/i_toplevel/self
run -all
