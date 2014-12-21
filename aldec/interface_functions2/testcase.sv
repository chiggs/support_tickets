module testcase();

logic clk, reset_n;

always
    #5 clk = ~clk;


initial begin
    clk = 1'b0;
    reset_n = 1'b0;
    mm_interface.read = 1'b1;
    #100 reset_n = 1'b1;
    #100 mm_interface.read = 1'b1;
    #110 mm_interface.read = 1'b0;
    #110 mm_interface.read = 1'b1;
    #100 $finish();
end


// interfaces
avalon_mm_lite                          mm_interface(
    .clk                                     (clk),
    .reset                                   (reset_n));

my_example i_example (
    .clk                                     (clk),
    .reset_n                                 (reset_n),
    .mm_interface                            (mm_interface.slave)
);

endmodule
