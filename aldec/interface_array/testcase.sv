module testcase #(
     parameter NUM_INTERFACES = 3
) (
     input clk,
     input [31:0] data,
     interface hash[NUM_INTERFACES-1:0]
);

always_ff @(posedge clk) begin
    for (int i=0; i<NUM_INTERACES; i++)
        hash[i].hash(data);
end

endmodule

