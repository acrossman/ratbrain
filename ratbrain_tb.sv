/*
 *    Izhikevich Neuron testbench
 */

`timescale 1ns/1ps

module izhikevich_neuron_tb;
    localparam CLK = 10; // period in ns, fq = 1/(CLK * 10^-9)
    localparam SAMPLE_WINDOW = 1000; // ms

    logic clk, reset;
    logic signed [31:0] I_in;
    logic signed [31:0] v_out;
    logic spike_out;

    izhikevich_neuron DUT (
        .clk(clk),
        .reset(reset),
        .I_in(I_in),
        .v_out(v_out),
        .spike_out(spike_out)
    );

    always begin
        clk = 1'b0;
        #(CLK/2); //clock low
        clk = 1'b1;
        #(CLK/2); //clock high
    end

    int spikes = 0;

    initial begin
        reset = 1'b0;
        I_in = 32'sd0;

        #10

        reset = 1'b1;
        I_in = 10 <<< 16;

        $display("Time(ms)\tv_out\tSpike");

        for (int i = 0; i < SAMPLE_WINDOW; i++) begin
            @(posedge clk)
            $display("%8d\t%32d\t%b", i+1, v_out, spike_out);
            spikes += spike_out;
        end

        $display("Test concluded. Total spikes: %d", spikes);

    end
endmodule





