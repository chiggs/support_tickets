

module my_example (
    input logic                                 clk,
    input logic                                 reset,

    // Statistics interface
    input logic                                 stat_valid,
    input logic                                 stat_reset,

    interface                                   mm_interface
);

typedef struct packed {
    logic some_value;
} state_t;

state_t self, next_self;

always_comb begin
     next_self = self;

     mm_interface.my_function(self.some_value);
end

always_ff @(posedge clk) begin
     self <= next_self;
end


endmodule
