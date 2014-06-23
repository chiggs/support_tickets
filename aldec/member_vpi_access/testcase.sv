package some_structs;

    typedef struct packed {
        logic                a_flag;
        logic [31:0]         a_vector;
    } struct1_t;

    typedef struct packed {
        struct1_t            a_substruct;
        logic                valid;
    } struct2_t;

endpackage


module some_module (
    input  some_structs::struct2_t            struct_input,
    output some_structs::struct2_t            struct_output
);


assign struct_output = struct_input;


endmodule


