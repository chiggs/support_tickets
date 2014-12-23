interface avalon_mm_lite #(
    parameter ADDRESS_WIDTH=4,
    parameter DATA_WIDTH=32
)(
    input clk,
    input reset
);

    logic                               read;
    logic                               waitrequest;

    modport slave (
        input                           clk,
        input                           reset,
        input                           read,
        output                          waitrequest,

        import function logic tick(),
        import function logic in_reset(),
        import function logic is_read()
    );

    // Functions, common for all memory-mapped style interfaces
    function logic is_read();
        $display("%0dns is_read() called", $time);
        return read & waitrequest;
    endfunction

    function logic tick();
        waitrequest     <= 1'b1;
        return 1'b0;
    endfunction

    function logic in_reset();
        waitrequest     <= 1'b1;
        return 1'b0;
    endfunction

endinterface
