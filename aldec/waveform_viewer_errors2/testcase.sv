package types_pkg;

    typedef logic [7:0] char_t;

    typedef logic unsigned [7:0]        uint8_t;
    typedef logic unsigned [15:0]       uint16_t;
    typedef logic unsigned [31:0]       uint32_t;
    typedef logic unsigned [63:0]       uint64_t;

endpackage


package lse_order_table;

typedef struct packed {
    types_pkg::uint64_t                 next;
    types_pkg::uint64_t                 previous;
    logic                               has_next;
    logic                               has_previous;
    logic                               adjusted;
    logic                               buy;
    logic [3:0]                         client;
    logic                               valid;
    logic [6:0]                         reserved;
    types_pkg::uint16_t                 instrument;
    types_pkg::uint32_t                 filled;
    types_pkg::uint32_t                 quantity;
    types_pkg::uint64_t                 price;
} order_entry_t;

endpackage


module toplevel #(
    parameter                                           MAX_CLIENTS             = 1,
    parameter                                           MAX_INSTRUMENTS         = 32,

    parameter                                           ORDER_TABLE_SIZE        = 1024,
    parameter                                           ORDER_TABLE_OVERHEAD    = 1024
) (

    input                                               clk,
    input                                               areset_n,
    input                                               doit
);


typedef struct packed {
    logic [$clog2(ORDER_TABLE_SIZE)-1:0]                address;
    lse_order_table::order_entry_t                      data;
    logic                                               rden;
    logic                                               wren;
} table_entry_t;


typedef struct packed {
    logic                                               pending_remove;
    logic                                               delete_ack;
    logic                                               handling_exchange;
    logic                                               handling_client;
    types_pkg::char_t   [19:0]                          current_order_id;
    logic [$clog2(ORDER_TABLE_SIZE)-1:0]                recycle_entry;
    logic                                               recycle_en;
    logic                                               lookup_en;
    logic                                               lookup_insert;
    table_entry_t                                       entry;
    logic                                               ack_order;
    logic                                               ack_update;
    logic [$clog2(MAX_CLIENTS)-1:0]                     current_client;
    logic [$clog2(MAX_INSTRUMENTS)-1:0]                 current_instrument;
    logic                                               positions_rden;
    logic                                               positions_wren;
} self_t;

self_t                                                  self, next_self;
logic [$bits(lse_order_table::order_entry_t)-1:0]       flattened_vector;

always_comb begin

    next_self  = self;

    // Defaults
    next_self.recycle_en        = 1'b0;
    next_self.lookup_en         = 1'b0;
    next_self.lookup_insert     = 1'b0;
    next_self.entry.rden        = 1'b0;
    next_self.entry.wren        = 1'b0;
    next_self.ack_order         = 1'b0;
    next_self.ack_update        = 1'b0;
    next_self.positions_rden    = 1'b0;
    next_self.positions_wren    = 1'b0;


                    // Completely new order - insert into our table!
    if (doit) begin
        next_self.entry.address             = '0;
        next_self.entry.data                = '0;
        next_self.entry.data.client         = '1;
        next_self.entry.data.instrument     = '0;
        next_self.entry.data.buy            = '1;
        next_self.entry.data.quantity       = '0;
        next_self.entry.data.price          = '1;
        next_self.entry.data.valid          = 1'b1;
        next_self.entry.wren                = 1'b1;

        next_self.lookup_insert             = 1'b1;
        next_self.ack_order                 = 1'b1;

        next_self.positions_rden            = 1'b1;
        next_self.current_client            = '0;
        next_self.current_instrument        = '1;
        next_self.handling_client           = 1'b1;
    end
end


always_ff @(posedge clk or negedge areset_n) begin

    if (~areset_n) begin
        self                    <= 'x;
        self.recycle_en         <= 1'b0;
        self.lookup_insert      <= 1'b0;
        self.recycle_entry      <= '0;
        self.ack_update         <= 1'b0;
        self.pending_remove     <= 1'b0;
    end else begin
        self                    <= next_self;
    end
end

assign flattened_vector = self.entry.data;

always @(posedge clk)
    $monitor("Flattened vector: %b", flattened_vector);

endmodule

module testcase();

logic clk, areset_n, doit;

    toplevel i_toplevel (.*);

always
    #5  clk =  ! clk; 


initial begin
    $dumpfile("testcase.vcd");
    $dumpvars(0,testcase);
    clk = 0;
    areset_n = 1;

    #10 areset_n = 0;
    #25 areset_n = 1;

    #20 doit = 1;



    #20
//     $display("/testcase/i_toplevel/self.entry.data.valid: %b but waveform has X", i_toplevel.self.entry.data.valid);
    $finish();
end

endmodule
