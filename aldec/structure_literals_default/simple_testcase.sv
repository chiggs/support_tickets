// Aldec Riviera-PRO bug
//
// Simple dummy module

package my_registers;

    typedef struct packed {
        logic [15:0]                    component;
        logic [3:0]                     major;
        logic [7:0]                     minor;
        logic [3:0]                     build;
    } version_register_t;

    typedef struct packed {
        logic [15:0]                    memory_offset;
        logic [7:0]                     stat_depth;
        logic [7:0]                     stat_width;
        version_register_t              version;
    } component_registers_t;


    // The following two structure literals should be equivalent?

    // This fails - version is defined as a single bit
    localparam component_registers_t reg_read_mask_fail = '{
        default:                        '1
    };

    // This works - version member is filled as expected
    localparam component_registers_t reg_read_mask_work = '{
        default:                        '1,
        version:                        '1
    };

    typedef logic [$bits(component_registers_t)-1:0] register_bits_t;
    localparam register_bits_t reg_read_mask_bits_fail       = reg_read_mask_fail;
    localparam register_bits_t reg_read_mask_bits_work       = reg_read_mask_work;

endpackage


module my_sub_module #(
    parameter                           REGISTER_BITS = 32,
    parameter logic [REGISTER_BITS-1:0] READ_MASK = 0
);

initial begin
    $display("Got READ_MASK = %b", READ_MASK);

    if (READ_MASK != (1<<REGISTER_BITS)-1)
        $display("FAILURE: Initialiser using default : '1 should be all 1s");
end


endmodule


module structure_literals_default ();

    my_sub_module #(
        .REGISTER_BITS ($bits(my_registers::component_registers_t)),
        .READ_MASK     (my_registers::reg_read_mask_bits_fail)
    ) i_testcase ();

    my_sub_module #(
        .REGISTER_BITS ($bits(my_registers::component_registers_t)),
        .READ_MASK     (my_registers::reg_read_mask_bits_work)
    ) i_testcase_work ();

endmodule

