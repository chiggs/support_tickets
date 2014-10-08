# Simple regression - make fails if bug is still present

.PHONY: all

all: runtest.do testcase.sv clean
	riviera -do runtest.do

clean:
	-rm -rf run.log
	-rm -rf work
	-rm -rf compile
	-rm -rf *.vcd
	-rm -rf library.cfg
	-rm -rf *.asdb
