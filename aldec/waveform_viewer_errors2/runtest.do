alib work
set worklib work
alog -dbg testcase.sv
asim  -dbg testcase
wave write
run -all
