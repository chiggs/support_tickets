adel -all
alib work

# compile project's source files
set TARGET libiterate

ccomp -pli -o $TARGET -dbg -D_DEBUG vpi_register.c
alog -dbg -pli $TARGET sample_module.v

# initialize simulation
asim -O2 -cdebug -pli $TARGET sample_module

run -all
quit
