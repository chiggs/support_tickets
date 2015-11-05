SOURCES = testcase.sv my_pkg.sv

.PHONY : quartus
quartus : $(SOURCES)
	quartus_map testcase

.PHONY : prime
prime: $(SOURCES)
	quartus_syn testcase

clean:
	-rm -rf testcase.syn.*
	-rm -rf *.summary
	-rm -rf testcase.qpf
	-rm -rf *db
	-rm -rf *clearbox
	-rm -rf testcase.*.rpt
