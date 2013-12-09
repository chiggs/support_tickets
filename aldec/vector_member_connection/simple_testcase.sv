// Aldec Riviera-PRO bug
//
// Fails to correctly resolve structures in port assignments if they are vectors
//

// Simple dummy module
module dummy_module (
    input  [3:0]         data_in,
    output [3:0]         data_out,
    output               data_out_valid
);

assign data_out = data_in;
assign data_out_valid = |data_in;

endmodule


// Actual testcase
module toplevel (
    input  [15:0]       data_in,
    output [15:0]       data_out
);

// FAILURE MODE
//
// We have a structure with a member "index" which we're connecting to a port
// inside a generate statement.
//
// Riviera complains that the:
// "Length of connection (1) does not match the length of port "data_out" (4)"
//
// However the index member is 4 bits wide, not a single bit.


typedef struct packed {
    logic [3:0]         index;
    logic               match;
} detect_t;

detect_t [3:0]          chars;




genvar g_char;
generate
    for (g_char=0; g_char<4; g_char=g_char+1) begin

        dummy_module i_dummy_failure (
            .data_in                           (data_in[g_char+:4]),
            .data_out                          (chars[g_char].index),           // FAILS
            .data_out_valid                    (chars[g_char].match)
        );
    end
endgenerate



// WORKAROUND
//
// Riviera can be bludgeoned into working however provided:
//
// 1. The width of the vector is *wider* than the port
//
// 2. We explicitly index a subslice of the member in the connection


typedef struct packed {
    logic [4:0]         index;          // 5 bits instead of 4
    logic               match;
} detect2_t;

detect2_t [3:0]          chars2;


genvar g_char2;
generate
    for (g_char2=0; g_char2<4; g_char2=g_char2+1) begin
        // Identical to above but explicity reference a slice of "index" member
        dummy_module i_dummy_working (
            .data_in                           (data_in[g_char2+:4]),
            .data_out                          (chars2[g_char2].index[3:0]),     // WORKS
            .data_out_valid                    (chars2[g_char2].match)
        );
    end
endgenerate


endmodule
