interface my_interface (
    input clk,
    input reset
);

    logic                               read;

    function void my_function();
        read <= 1'b0;
    endfunction

    modport master (
        output  read
    );

    modport slave (
        input                           read,

        import function void  my_function()
    );

endinterface
