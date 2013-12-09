# Simple regression - make fails if bug is still present

.PHONY: all

all: runtest.do simple_testcase.sv clean
	vsimsa -do runtest.do | tee run.log
	@if [[ `grep -c "does not match the length of port" run.log` -ne 0 ]];\
		then echo "Testcase still fails!";\
		exit 1;\
	else\
		echo "Testacse passed!";\
	fi

clean:
	-rm -rf run.log
