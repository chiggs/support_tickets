
package ufo_protocol_pkg;

    // Message types client -> exchange
    typedef enum logic [7:0] {
        UFO_LOGIN_ACCEPT_E                  = "A",
        UFO_LOGIN_REJECT_E                  = "J",
        UFO_SEUQNCED_DATA_E                 = "S",
        UFO_END_OF_SESSION_E                = "E"
    } exchange2client_types_t;

    // Message types client -> exchange
    typedef enum logic [7:0] {
        UFO_LOGIN_REQUEST_E                 = "L",
        UFO_RETRANSMISSION_REQUEST_E        = "T",
        UFO_UNSEQUENCED_DATA_E              = "U",
        UFO_HEARTBEAT_E                     = "R",
        UFO_LOGOFF_REQUEST_E                = "O"
    } client2exchange_types_t;


    typedef struct packed {
        logic [15:0]                        message_count;
        logic [31:0]                        sequence_number;
        exchange2client_types_t             msgtype;
    } sequenced_data_t;

endpackage: ufo_protocol_pkg



module testcase #(
    parameter                                           UFO_DATA_WIDTH  = 64,
    parameter                                           OUCH_DATA_WIDTH = 64,
    parameter                                           MAX_SESSIONS    = 512
) (
    input                                               clk,
    input                                               areset_n,

    input [UFO_DATA_WIDTH-1:0]                          ufo_in_data,
    input                                               ufo_in_startofpacket,
    input                                               ufo_in_endofpacket,
    input [$clog2(UFO_DATA_WIDTH/8)-1:0]                ufo_in_empty,
    input                                               ufo_in_valid,
    input [$clog2(MAX_SESSIONS)-1:0]                    ufo_in_channel,
    output                                              ufo_in_ready,

    output logic [UFO_DATA_WIDTH-1:0]                   ouch_out_data,
    output logic                                        ouch_out_startofpacket,
    output logic                                        ouch_out_endofpacket,
    output logic [$clog2(UFO_DATA_WIDTH/8)-1:0]         ouch_out_empty,
    output logic                                        ouch_out_valid,
    input                                               ouch_out_ready
);




typedef enum integer unsigned {
    IDLE,
    STATE_LOOKUP,
    HANDLE_PACKET,
    LOGIN_ACCECPT_SKIP_SESSION,
    SEQUENCED_MESSAGE_NMSGS,
    FORWARDING,
    DROPPING
} state_enum_e;

typedef struct packed {
    logic [4:0]                                         consume_bytes;
    logic [7:0]                                         current_offset;
} state_t;


state_enum_e                                            state, next_state;
state_t                                                 r, next_r;

ufo_protocol_pkg::sequenced_data_t                      ufo_header;

assign ufo_header = ufo_in_data[UFO_DATA_WIDTH-1:UFO_DATA_WIDTH-$bits(ufo_header)];

always_comb begin

    next_state = state;
    next_r = r;

    // Defaults
    next_r.consume_bytes        = '0;

    case (state)

        IDLE: begin
            next_state                      = STATE_LOOKUP;
        end

        HANDLE_PACKET: begin

            // WARNING: Aldec currently barfs on nested case statements?
            case (ufo_header.msgtype)

                ufo_protocol_pkg::UFO_END_OF_SESSION_E: begin
                    next_state                      = DROPPING;
                end

                default: begin
                end
            endcase
        end
    endcase
end

always_ff @(posedge clk or negedge areset_n) begin

    if (~areset_n) begin
        r                       <= 'x;
        state                   <= IDLE;
    end else begin
        state                   <= next_state;
        r                       <= next_r;
    end
end


endmodule : testcase