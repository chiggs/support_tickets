# Simple regression - make fails if bug is still present

.PHONY: all

all: $(wildcard *.sv) clean
	vsimsa -do runsim.tcl | tee run.log
	@if [[ `grep -c "Error" run.log` -ne 0 ]];\
		then echo "Testcase still fails!";\
		exit 1;\
	else\
		echo "Testacse passed!";\
	fi

clean:
	-rm -rf run.log
	-rm -rf work
	-rm -rf compile
	-rm -rf library.cfg
	-rm -rf dataset.asdb
	-rm -rf work.acdb
