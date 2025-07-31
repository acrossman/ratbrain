/*
 *    Izhikevich Neuron implementation in hardware.
 *
 *    @author: Crow Crossman
 *    @date: 22 July 2025
 */

module izhikevich_neuron #(
    parameter logic signed [31:0] A = 32'sd1311, // 0.02
    parameter logic signed [31:0] B = 32'sd13107, // 0.2
    parameter logic signed [31:0] C = -65 <<< 16,
    parameter logic signed [31:0] D = 8 <<< 16
)(
     input logic clk, reset,
     input logic signed [31:0] I_in,
     output logic signed [31:0] v_out,
     output logic spike_out
    );

    localparam logic signed [31:0] quadratic_co = 32'sd2621; //.04
    localparam logic signed [31:0] linear_co = 5 <<< 16;
    localparam logic signed [31:0] constant_co = 140 <<< 16;
    localparam logic signed [31:0] voltage_threshold = 30 <<< 16;

    logic signed [31:0] v, u;
    logic signed [31:0] v_next, u_next;
    logic fired;

    always_comb begin
        logic signed [63:0] quadratic_mult;
        logic signed [63:0] linear_mult;
        logic signed [63:0] u64_tmp;
        logic signed [63:0] dv64_tmp, du64_tmp;
        logic signed [31:0] dv, du;


        if (v >= voltage_threshold) begin
            fired = 1'b1;
            v_next = C;
            u_next = u + D;   
        end else begin
            fired = 1'b0;
            quadratic_mult = quadratic_co * ((v * v) >>> 16);
            linear_mult = linear_co * v;
            
            dv64_tmp = (quadratic_mult >>> 16) + (linear_mult >>> 16) + constant_co - u + I_in;
            dv = dv64_tmp;

            u64_tmp = B * v;
            du64_tmp = A * ((u64_tmp >>> 16) - u);
            du = du64_tmp >>> 16;

            v_next = v + dv;
            u_next = u + du;   
        end
    end

    always_ff @(posedge clk or negedge reset) begin
        if (!reset) begin
            v <= C;
            u <= (B * C) >>> 16;
            spike_out <= 1'b0;
        end else begin
            v <= v_next;
            u <= u_next;
            spike_out <= fired;
        end
    end

    assign v_out = v;

endmodule

