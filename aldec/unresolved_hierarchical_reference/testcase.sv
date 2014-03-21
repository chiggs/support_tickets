package my_package;


    typedef struct packed {
        logic        my_member;
    } my_struct;

endpackage


module my_test();

    my_package::my_struct my_inst;

    initial begin
       my_inst.not_a_member = 1;
    end

endmodule
