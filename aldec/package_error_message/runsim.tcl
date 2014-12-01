alib work
set worklib work
alog my_pkg.sv my_module.sv
asim  +access +w -O2 -dbg my_module
run -all
endsim

