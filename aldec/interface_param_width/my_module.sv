// Simple matching engine.
//
// Counts occurances of a particular string in received packets

module my_module (
    input                                       clk,
    input                                       reset,

    interface                                   stream_in
);

localparam WIDTH_1                            = stream_in.DATA_WIDTH;
localparam WIDTH_2                            = $bits(stream_in.data);

initial begin
    $display("WIDTH_1: %d", WIDTH_1);
    $display("WIDTH_2: %d", WIDTH_2);
    #100 $finish(0);
end

endmodule


