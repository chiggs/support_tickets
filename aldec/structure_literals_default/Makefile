# Simple regression - make fails if bug is still present

.PHONY: all

all: runtest.do simple_testcase.sv clean
	vsimsa -do runtest.do | tee run.log

clean:
	-rm -rf dataset.asdb
	-rm -rf work
	-rm -rf compile
	-rm -rf library.cfg
	-rm -rf run.log
