// Quartus doesn't seem to be able to handle structure literals

package my_pkg;

typedef struct packed {
    logic [31:0]    some_value;
    logic           some_flag;
} my_struct_t;

localparam my_struct_t constant_value = '{
    some_value:    '1,
    default:       '1
};

localparam logic [$bits(my_struct_t)-1:0] constant_value_bits = constant_value;

endpackage: my_pkg


