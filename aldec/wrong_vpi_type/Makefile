all: verilog

verilog: clean
	vsimsa -do runme.do

clean:
	-$(RM) -rf work
	-$(RM) -rf dataset.*
	-$(RM) -rf *.vcd *.so *.cfg compile
	
