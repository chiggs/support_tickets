module my_module #(
    parameter                                   DATA_WIDTH=32
) (
    input                                       clk,
    input                                       reset,
    input  [DATA_WIDTH-1:0]                     stream_in_data,
    input                                       stream_in_valid
);

my_pkg::parse_state_t                             self, next_self;



always_comb begin

    next_self = self;

    // Doesn't work?
    if (stream_in_valid)
        my_pkg::parse_32(stream_in_data[31:0], next_self);

    // Works?
    if (stream_in_valid)
        next_self.working_checksum = self.working_checksum + stream_in_data[15:0];

end

always_ff @(posedge clk)
    self        <= next_self;

endmodule
