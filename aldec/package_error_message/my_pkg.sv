package my_pkg;

    typedef struct packed {
        logic [15:0]                    working_checksum;
    } parse_state_t;

    localparam parse_state_t reset_state = '{
        working_checksum:               '0
    };

    function automatic void parse_32(input logic [31:0] data_in, inout parse_state_t self);
        self.working_checksum           = self.working_checksum + data_in[15:0];
    endfunction

endpackage

