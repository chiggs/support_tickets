all: verilog

verilog: clean
	vsimsa -do runme.do

mixed: clean
	vsimsa -do runme_mixed.do

clean:
	-$(RM) -rf work
	-$(RM) -rf dataset.*
	-$(RM) -rf *.vcd *.so *.cfg compile
	
