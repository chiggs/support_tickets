`timescale 1 ps / 1 ps

module sample_module (
    input                                       clk,

    output reg                                  stream_in_ready,
    input                                       stream_in_valid,
`ifndef __ICARUS__
    input real                                  stream_in_real,
    input  integer                              stream_in_int,
`endif
    input  [7:0]                                stream_in_data,
    input  [63:0]                               stream_in_data_wide,

    input                                       stream_out_ready,
`ifndef __ICARUS__
    output real                                 stream_out_real,
    output integer                              stream_out_int,
`endif
    output reg                                  stream_out_valid,
    output reg [7:0]                            stream_out_data_comb,
    output reg [7:0]                            stream_out_data_registered
);

always @(posedge clk)
    stream_out_data_registered <= stream_in_data;

always @(stream_in_data)
    stream_out_data_comb = stream_in_data;

always @(stream_in_data)
    stream_out_data_comb = stream_in_data;

always @(stream_out_ready)
    stream_in_ready      = stream_out_ready;

`ifndef __ICARUS__
always @(stream_in_real)
    stream_out_real      = stream_in_real;

always @(stream_in_int)
    stream_out_int <= stream_in_int;
`endif

`ifdef INST_VHDL
vhdl_entity i_vhdl_entity (
    .clk                 (clk),
    .reset_n             (stream_out_ready),
    .stream_out_valid    (stream_out_valid)
);
`endif

endmodule
