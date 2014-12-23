

module my_example (
    input logic                                 clk,
    input logic                                 reset_n,

    interface                                   mm_interface
);


always_comb begin: local_variable

    logic is_read;

    is_read = mm_interface.is_read();

    if (is_read)
        $display("%0dns is_read() true in local_variable process", $time);
    else
        $display("%0dns local_variable process scheduled", $time);
end


always_comb begin: direct_call
    if (mm_interface.is_read())
        $display("is_read() true in direct_call process", $time);
    else
        $display("%0dns direct_call process scheduled", $time);
end


always_ff @(posedge clk, negedge reset_n) begin
    if (~reset_n) begin
        mm_interface.in_reset();
    end else begin
        mm_interface.tick();
    end
end


endmodule
