package types_pkg;

    typedef logic [7:0] char_t;

    typedef enum char_t {
        MSG_ENTER_ORDER                 = "O",
        MSG_REPLACE_ORDER               = "U",
        MSG_CANCEL_ORDER                = "X",
        MSG_MODIFY_ORDER                = "M"
    } msg_type_e;

endpackage


package config_pkg;


    typedef struct packed {
        logic [7:0]                     msg_type;
        logic [31:0]                    version;
    } component_registers_t;

    localparam component_registers_t reg_read_mask = '{
        default:                        '1
    };

    localparam component_registers_t reg_write_mask = '{
        version:                        32'h00000000,
        default:                        '1
    };

    localparam component_registers_t reg_reset_value = '{
        version:                        32'hx,
        msg_type:                       types_pkg::MSG_ENTER_ORDER
    };

endpackage


