
.PHONY: all working failing

all: working failing

failing: stack.sv runsim_failing.tcl
	vsimsa -do runsim_failing.tcl | tee failing.log

working: stack.sv runsim_working.tcl
	vsimsa -do runsim_working.tcl | tee working.log

clean:
	-rm *.log
	-rm -rf compile
	-rm -rf failing_example
	-rm -rf working_example
	-rm library.cfg

