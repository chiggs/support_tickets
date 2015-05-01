module testcase ();

logic clk;
logic reset;
logic [63:0] data;

// interfaces
my_interface #(
     .DATA_WIDTH                             (64)
) stream_in (
    .clk                                     (clk),
    .reset                                   (reset),
    .data                                    (data)
);


my_module i_my_module (
    .clk                                     (clk),
    .reset                                   (reset),
    .stream_in                               (stream_in)
);

endmodule

