.PHONY: all libs sim clean valgrind
all: sim

build :
	mkdir build

build/runsim.do: vpi_members.c |build
	echo "alib work" > $@
	echo "set worklib work" >> $@
	echo "ccomp -verbose -dbg -pli -o libvpi_members.so ../vpi_members.c" >> $@
	echo "alog -dbg ../testcase.sv" >> $@
	echo "asim $(ASIM_ARGS) +access +w +O2 -dbg -pli libvpi_members  some_module" >> $@
	echo "run 20ms" >> $@
	echo "endsim" >> $@


sim: build/runsim.do
	cd build && vsimsa -do runsim.do | tee sim.log

clean::
	rm -rf build

