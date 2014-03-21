.PHONY: all sim clean

all: sim

sim: runme.do testcase.sv
	vsimsa -do runme.do

clean:
	-rm -rf work
	-rm -rf compile
	-rm library.cfg
