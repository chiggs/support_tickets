
module testcase #(
        parameter                                   TABLE_SIZE              = 64,
        parameter                                   ENTRIES_PER_BUCKET      = 2,
        parameter                                   NUM_TABLES              = 2,

        parameter                                   CAM_ENTRIES             = 32,

        parameter                                   KEY_WIDTH               = 8,
        parameter                                   VALUE_WIDTH             = 12,

        // Behaviour modifiers
        parameter                                   MAX_EVICTION_CHAIN      = 10
        ) (
        input                                       areset_n,
        input                                       clk,

        // Lookup interface
        input  [KEY_WIDTH-1:0]                      lookup_key,
        input                                       lookup_en,

        output logic                                insert_ready
        );


    typedef logic [KEY_WIDTH-1:0]                   key_t;
    typedef logic [VALUE_WIDTH-1:0]                 value_t;



    typedef struct packed {
        key_t                                       key;
        value_t                                     value;
        logic                                       valid;
    } bucket_entry_t;

    typedef bucket_entry_t [0:ENTRIES_PER_BUCKET-1] ram_data_t;
    typedef logic [$clog2(TABLE_SIZE)-1:0]          ram_address_t;

    genvar i;
    generate
        for (i=0; i<NUM_TABLES; i++)  begin: i_ram
            initial $display("FAILING: ram_address_t bits %d (expected 6)  and high is %d (expected 5)", $bits(ram_address_t), $high(ram_address_t));
            initial $display("FAILING: ram_data_t    bits %d (expected 42) and high is %d (expected 1)", $bits(ram_data_t), $high(ram_data_t));
        end
    endgenerate

    initial $display("CORRECT: ram_data_t    bits %d (expected 42) and high is %d (expected 1)", $bits(ram_data_t), $high(ram_data_t));
    initial $display("CORRECT: ram_address_t bits %d (expected 6)  and high is %d (expected 5)", $bits(ram_address_t), $high(ram_address_t));

endmodule
