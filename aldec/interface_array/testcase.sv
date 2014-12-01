module testcase #(
     parameter NUM_INTERFACES = 3
) (
     input clk,
     input [31:0] data,
     interface hash[NUM_INTERFACES-1:0]
);

logic [NUM_INTERFACES-1:0] result;

always_ff @(posedge clk) begin
    for (int i=0; i<NUM_INTERFACES; i++)
        result[i] = hash[i].hash(data);
end

endmodule

