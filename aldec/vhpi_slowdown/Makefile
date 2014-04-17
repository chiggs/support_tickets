ifeq ($(GUI),1)
    CMD := riviera -nosplash
else
    CMD := vsimsa
endif


.PHONY: all libs sim clean
all: sim

build :
	mkdir build

#build/vhpi_example.o: vhpi_example.c | build
#	cd build && gcc -I$(ALDEC_PATH)/interfaces/include -DVPI_CHECKING -c -fpic -o vhpi_example.o ../vhpi_example.c

#build/libexample.so: build/vhpi_example.o
#	cd build && gcc -L$(ALDEC_PATH)/bin/Linux64 -laldecpli -lstdc++ -lsupc++ -o libexample.so vhpi_example.o

build/runsim.do: vhpi_example.c |build
	echo "alib work" > $@
	echo "set worklib work" >> $@
	echo "ccomp -verbose -dbg -vhpi -o libexample.so ../vhpi_example.c" >> $@
	echo "acom -dbg ../vhpi_example.vhdl" >> $@
	echo "asim $(ASIM_ARGS) +access +w +O2 -dbg -loadvhpi libexample:vhpi_startup_routines_bootstrap vhpi_example" >> $@
	echo "run 10ms" >> $@
	echo "endsim" >> $@


sim: build/runsim.do
	cd build && $(CMD) -do runsim.do | tee sim.log


clean::
	rm -rf build

