package example_register_pkg;

    typedef struct packed {
        logic [15:0]                    component;
        logic [3:0]                     major;
        logic [7:0]                     minor;
        logic [3:0]                     build;
    } version_register_t;

    localparam version_register_t version = '{
        component:                      16'hDA7A,
        major:                          4'd1,
        minor:                          8'd2,
        build:                          4'd0
    };

    typedef struct packed {
        logic [31:0]                    sequence_number;        // Read-only, for debug purposes
        logic [15:0]                    event_buffer;           // The ID off the buffer containing a subscription even message
        logic [7:0]                     num_buffers;            // Read-only, compile-time constant
        logic [5:0]                     reserved_0;
        logic                           acknowledge_event;      // Event buffer latched - can perform another subscription
        logic                           enabled;
        logic [31:0]                    client_subscription;    // Client subscription bitmask -- write 1 to set, read for value (FIXME?)
        logic [31:0]                    physical_address_base;  // Physical address for this pool
    } dma_pool_descriptor_t;

    localparam dma_pool_descriptor_t descriptor_read_mask = '{
        reserved_0:                     '0,
        default:                        '1
    };

    localparam dma_pool_descriptor_t descriptor_write_mask = '{
        physical_address_base:          '1,
        client_subscription:            '1,
        enabled:                        '1,
        acknowledge_event:              '1,
        default:                        '0
    };

    localparam dma_pool_descriptor_t descriptor_pulse_mask = '{
        client_subscription:            '1,
        default:                        '0
    };

    localparam dma_pool_descriptor_t descriptor_reset_value = '{
        enabled:                        1'b0,
        acknowledge_event:              1'b0,
        client_subscription:            32'h01234567,
        default:                        'x
    };


    typedef struct packed {
        dma_pool_descriptor_t [3:0]     pools;
        logic [6:0]                     reserved;               // Padding to 32-bit align
        logic                           subscription_ready;     // Assumes only one controlling process for simplicity
        logic [15:0]                    page_size;              // Read only constant
        logic [7:0]                     num_channels;           // Read only constant
        version_register_t              version;
    } component_registers_t;

    typedef logic [$bits(component_registers_t)-1:0] register_bits_t;

    localparam component_registers_t reg_read_mask = '{
        reserved:                       '0,
        pools:                          '{4{descriptor_read_mask}},
        default:                        '1
    };

    localparam component_registers_t reg_write_mask = '{
        pools:                          '{4{descriptor_write_mask}},
        default:                        '0
    };

    localparam component_registers_t reg_pulse_mask = '{
        pools:                          '{4{descriptor_pulse_mask}},
        default:                        '0
    };

    localparam component_registers_t reg_reset_value = '{
        pools:                          '{4{descriptor_reset_value}},
        default:                        'x
    };


    // Required for Quartus - can't pass structs through parameters, d'oh
    localparam register_bits_t reg_read_mask_bits       = reg_read_mask;
    localparam register_bits_t reg_write_mask_bits      = reg_write_mask;
    localparam register_bits_t reg_pulse_mask_bits      = reg_pulse_mask;
    localparam register_bits_t reg_reset_value_bits     = reg_reset_value;


localparam ADDRESS_BITS                 = $clog2((($bits(component_registers_t)+31) / 32) * 4);

endpackage


module register_map #(
    parameter                           REGISTER_BITS           = $bits(example_register_pkg::component_registers_t),
    parameter logic [REGISTER_BITS-1:0] REGISTER_RESET_VALUE    = example_register_pkg::reg_reset_value_bits,
    parameter logic [REGISTER_BITS-1:0] REGISTER_READABLE_MASK  = example_register_pkg::reg_read_mask_bits,
    parameter logic [REGISTER_BITS-1:0] REGISTER_WRITEABLE_MASK = example_register_pkg::reg_write_mask_bits,
    parameter logic [REGISTER_BITS-1:0] REGISTER_PULSE_MASK     = example_register_pkg::reg_pulse_mask_bits
) (
    input                               clk,
    input                               areset_n,

    // Interface to user logic
    output [REGISTER_BITS-1:0]          writeable_registers,
    input  [REGISTER_BITS-1:0]          readable_registers
);

logic [REGISTER_BITS-1:0]               register_bits_r;
logic [REGISTER_BITS-1:0]               external_read_registers;
logic [REGISTER_BITS-1:0]               internal_write_registers;

always_ff @(posedge clk or negedge areset_n) begin

    if (~areset_n) begin
        register_bits_r                 <= REGISTER_RESET_VALUE;
    end else begin
        external_read_registers         = (readable_registers & REGISTER_READABLE_MASK & (~REGISTER_WRITEABLE_MASK | REGISTER_PULSE_MASK));
        internal_write_registers        = register_bits_r & REGISTER_WRITEABLE_MASK & ~REGISTER_PULSE_MASK;
        register_bits_r                 <= (external_read_registers | internal_write_registers) & REGISTER_WRITEABLE_MASK;
    end
end

assign writeable_registers = register_bits_r | (readable_registers & ~REGISTER_WRITEABLE_MASK);

endmodule

module testcase();

logic clk, areset_n;
example_register_pkg::component_registers_t write, read;

register_map i_map(
    .clk                        (clk),
    .areset_n                   (areset_n),
    .writeable_registers        (write),
    .readable_registers         (read)
    );


always
    #5  clk =  ! clk; 

initial begin

    $dumpfile("testcase.vcd");
    $dumpvars(0,testcase);

    clk = 0;
    areset_n = 1;
    read.version                = example_register_pkg::version;

    #10 areset_n = 0;
    #25 areset_n = 1;

    #20
    $display("write.pools[0].acknowledge_event: %b but waveform has 1", write.pools[0].acknowledge_event);
    $display("write.pools[1].acknowledge_event: %b but waveform has 1", write.pools[1].acknowledge_event);
    $display("write.pools[3].acknowledge_event: %b but waveform has 1", write.pools[3].acknowledge_event);
    $display("write.pools[2].enabled: %b but waveform has 1", write.pools[2].enabled);
    $display("write.pools[3].enabled: %b but waveform has 1", write.pools[2].enabled);
    #10 $finish();

end

endmodule







