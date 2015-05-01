// Interface encapsulating an avalon_st sink.
//
// We provide a common set of functions that will work on other bus formats.
// For example even though avalon doesn't support sparsely populated words
// (null bytes in a packet) we include functions to check whether a byte is
// valid.
//
// Note Quartus doesn't currently support modports so we end up with a
// separate interface for source / sink.
//

interface my_interface #(
    parameter                                           DATA_WIDTH      = 32
)(
    input  logic                                        clk,
    input  logic                                        reset,

    input  logic [DATA_WIDTH-1:0]                       data
);

    function logic [DATA_WIDTH-1:0] get_data();
        return data;
    endfunction

endinterface

