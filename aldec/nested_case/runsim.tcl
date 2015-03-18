alib work
set worklib work
alog -dbg -coverage sb testcase.sv
asim -acdb +access +w -O2 -dbg testcase
run -all
endsim
