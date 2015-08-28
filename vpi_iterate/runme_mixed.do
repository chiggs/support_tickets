adel -all
alib work

# compile project's source files
set TARGET libiterate

ccomp -pli -o $TARGET -dbg -D_DEBUG vpi_iterate.c
vcom -dbg vhdl_module.vhdl
alog -dbg -pli $TARGET +define+INST_VHDL sample_module.v

# initialize simulation
asim -O2 -cdebug -pli $TARGET sample_module

run -all
quit
