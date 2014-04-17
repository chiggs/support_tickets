.PHONY: all libs sim clean valgrind
all: sim

build :
	mkdir build

build/runsim.do: vhpi_example.c |build
	echo "alib work" > $@
	echo "set worklib work" >> $@
	echo "ccomp -verbose -dbg -vhpi -o libexample.so ../vhpi_example.c" >> $@
	echo "acom -dbg ../vhpi_example.vhdl" >> $@
	echo "asim $(ASIM_ARGS) +access +w +O2 -dbg -loadvhpi libexample:vhpi_startup_routines_bootstrap vhpi_example" >> $@
	echo "run 10ms" >> $@
	echo "endsim" >> $@


sim: build/runsim.do
	cd build && vsimsa -do runsim.do | tee sim.log

valgrind: build/runsim.do
	cd build && valgrind --trace-children=yes --leak-check=full vsimsa -do runsim.do

clean::
	rm -rf build

