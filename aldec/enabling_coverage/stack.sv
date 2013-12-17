module stack #(
    parameter                                   WIDTH                   = 32,
    parameter                                   DEPTH                   = 4     // Must be a power of two
) (
    input                                       clk,
    input                                       reset_n,

    input [WIDTH-1:0]                           data_in,
    input                                       push,
    input                                       pop,

    output [WIDTH-1:0]                          data_out,
    output                                      empty,
    output                                      full
);


typedef struct packed {
    logic                                       full;
    logic                                       empty;
    logic [$clog2(DEPTH)-1:0]                   read_address;
    logic [$clog2(DEPTH)-1:0]                   write_address;
} status_t;

status_t status;
logic   wren;

// Read address is always write_address - 1;
// For optimal Fmax we duplicate the register rather than combinatorially
// deriving the read or write address.
always_ff @(posedge clk, negedge reset_n) begin
    if (!reset_n) begin
        status.full              <= 1'b0;
        status.empty             <= 1'b1;
        status.read_address      <= { $clog2(DEPTH) { 1'b1} };
        status.write_address     <= '0;

    end else begin

        if (push & !pop & !status.full) begin

            status.empty         <= 1'b0;
            status.write_address <= status.write_address + 1'b1;
            status.read_address  <= status.read_address + 1'b1;

            if (status.write_address == DEPTH-1)
                status.full      <= 1'b1;

        end else if (pop & !push & !status.empty) begin

            status.full          <= 1'b0;
            status.write_address <= status.write_address - 1'b1;
            status.read_address  <= status.read_address - 1'b1;

            if (!status.read_address)
                status.empty     <= 1'b1;

        end
    end
end

assign full     = status.full;
assign empty    = status.empty;
assign wren     = push & !pop & !status.full;


// Instantiate the Altera RAM
altdpram #(
    .indata_aclr                                ("OFF"),
    .indata_reg                                 ("INCLOCK"),
    .intended_device_family                     ("Stratix V"),
    .lpm_type                                   ("altdpram"),
    .outdata_aclr                               ("OFF"),
    .outdata_reg                                ("UNREGISTERED"),
    .ram_block_type                             ("MLAB"),
    .rdaddress_aclr                             ("OFF"),
    .rdaddress_reg                              ("UNREGISTERED"),
    .rdcontrol_aclr                             ("OFF"),
    .rdcontrol_reg                              ("UNREGISTERED"),
    .read_during_write_mode_mixed_ports         ("DONT_CARE"),
    .width                                      (WIDTH),
    .widthad                                    ($clog2(DEPTH)),
    .width_byteena                              (1),
    .wraddress_aclr                             ("OFF"),
    .wraddress_reg                              ("INCLOCK"),
    .wrcontrol_aclr                             ("OFF"),
    .wrcontrol_reg                              ("INCLOCK")
) i_stack_ram (
    .data                                       (data_in),
    .outclock                                   (),
    .rdaddress                                  (status.read_address),
    .wren                                       (wren),
    .inclock                                    (clk),
    .wraddress                                  (status.write_address),
    .q                                          (data_out),
    .aclr                                       (1'b0),
    .byteena                                    (1'b1),
    .inclocken                                  (1'b1),
    .outclocken                                 (1'b0),
    .rdaddressstall                             (1'b0),
    .rden                                       (1'b1),
    .wraddressstall                             (1'b0)
);

endmodule
