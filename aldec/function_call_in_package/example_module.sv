module example_module #(
    parameter                                   DATA_WIDTH=64
) (
    input                                       clk,
    input                                       reset,

    input  [DATA_WIDTH-1:0]                     stream_in_data
);

always_comb begin
    example_pkg::parse_32(stream_in_data[31:0]);
end

endmodule