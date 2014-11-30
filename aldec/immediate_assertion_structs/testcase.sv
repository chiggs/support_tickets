module testcase();

typedef struct packed {
    logic [1:0]    something;
} substruct_t;

typedef struct packed {
    logic          a_bit;
    logic          another_bit;
    substruct_t    substruct;
} my_struct_t;

typedef struct packed {
    logic [1:0]   two_bits;
    substruct_t   substruct;
} my_other_struct_t;

my_struct_t my_struct;
my_other_struct_t my_other_struct;


initial begin

    my_struct = 4'b1010;
    my_other_struct = 4'b1010;

    if (my_struct == 4'b1010)
        $display("my_struct == 4'b1010");
    else
        $display("my_struct != 4'b1010");

    if (my_struct == my_other_struct)
        $display("if: my_struct == my_other_struct");
    else
        $display("if: my_struct != my_other_struct");

    assert (my_struct == 4'b10)
        $display("assert: my_struct == 4'b1010");
    else
        $display("assert: my_struct != 4'b1010");
        
    assert (my_struct == my_other_struct)
        $display("assert: my_struct == my_other_struct");
    else
        $display("assrt: my_struct != my_other_struct");

    #10 $finish(0);
end


endmodule

