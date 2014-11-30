module testcase();


always_comb begin

    // This for loop uses undeclared variable i
    // should be for (int i=0; ...
    for (i=0; i<4; i++)
        $display("This is an erroneous for loop");
end

endmodule
