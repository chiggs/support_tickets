module example (
    input                                       areset_n,
    input                                       clk,

    output logic [7:0]                          something
);




config_pkg::component_registers_t               write;

always_ff @(posedge clk) begin
    something <= write.msg_type;
end

endmodule
