// Quartus doesn't seem to be able to handle structure literals

module my_module #(
    parameter                          NUM_BITS = 0,
    parameter logic [NUM_BITS-1:0]     TOGGLE_VALUE
) (
    input  logic                       clk,
    output logic [NUM_BITS-1:0]        q
);

logic flag = 1'b0;

always_ff @(posedge clk) begin

    flag <= ~flag;
    if (flag)
        q <= TOGGLE_VALUE;
    else
        q <= '0;
end

endmodule


module testcase (
    input  logic                       clk,
    output my_pkg::my_struct_t         q
);

my_module #(
    .NUM_BITS                          ($bits(my_pkg::constant_value_bits)),
    .TOGGLE_VALUE                      (my_pkg::constant_value_bits)
) i_module (
    .clk                               (clk),
    .q                                 (q)
);

endmodule

