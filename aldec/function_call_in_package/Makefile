# Simple regression - make fails if bug is still present

.PHONY: all

all: runtest.do example_pkg.sv example_module.sv clean
	vsimsa -do runtest.do | tee run.log
	@if [[ `grep -c "Fatal Error" run.log` -ne 0 ]];\
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
