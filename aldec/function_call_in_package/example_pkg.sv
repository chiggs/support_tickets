package example_pkg;

    function void parse_32(input logic [31:0] data_in);
        logic [15:0] dst;
        dst = data_in[15:0];
    endfunction

endpackage

