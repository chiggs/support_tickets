package types_pkg;

    typedef logic unsigned [31:0]       uint32_t;
    typedef logic unsigned [63:0]       uint64_t;

endpackage

package my_pkg;

    typedef struct packed {
        types_pkg::uint32_t             uint32_signal;
        types_pkg::uint64_t[2:0]        uint64_array;
        types_pkg::uint64_t             uint64_signal;
        logic [2:0]                     logic_array;
        logic unsigned [31:0]           uint32_signal_local;
        logic unsigned [63:0]           uint64_signal_local;
    } config_t;

endpackage

module testcase();
    my_pkg::config_t                    normal_config;
endmodule
