# Simple regression - make fails if bug is still present

.PHONY: all

all: runtest.do testcase.sv clean
	vsimsa -do runtest.do | tee run.log
	@if [[ `grep -c "FAILED" run.log` -ne 0 ]];\
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
