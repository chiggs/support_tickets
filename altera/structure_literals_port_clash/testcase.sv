// Quartus gets confused by the structure literal because members have the
// same name as ports.

module structure_literals_testcase (
    input           data_in,
    output          data_out
);

typedef struct packed {
    logic          data_in;
    logic          data_out;
    logic          something_else;
} my_struct_t;

localparam my_struct_t struct_literal = '{
    data_in:       1'b1,
    data_out:      1'b0,
    default:       'x
};

my_struct_t        my_struct;


assign my_struct = struct_literal;

assign data_out = my_struct.data_out;

endmodule

