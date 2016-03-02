adel -all
alib work

# compile project's source files
set TARGET libvpi_types

ccomp -pli -o $TARGET -dbg -D_DEBUG vpi_find_types.c
alog -dbg -pli $TARGET testcase.sv

# initialize simulation
asim -O2 -cdebug -pli $TARGET testcase

run -all
quit
