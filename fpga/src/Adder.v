`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/10/2025 10:20:07 PM
// Design Name: 
// Module Name: Adder
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module Adder (
    input clk,
    input reset,
    input  [96:0] a,
    input  [96:0] b,
    output  reg [96:0] o,
    output reg en
);
    wire [95:0] out;
    reg [96:0] delayed_a;
    fp32_add add_x(.a(delayed_a[0+:32]),.b(b[0+:32]),.o(out[0+:32]),.sub(0));
    fp32_add add_y(.a(delayed_a[32+:32]),.b(b[32+:32]),.o(out[32+:32]),.sub(0));
    fp32_add add_z(.a(delayed_a[64+:32]),.b(b[64+:32]),.o(out[64+:32]),.sub(0));
    //assign en = (a[96] == 1'b1 || b == 1'b1) ? 1'b0 : 1'b1;
    always @(posedge clk) begin
        if(reset || (a[96])) begin
            delayed_a <= {1'b1,{96{1'b0}}};
        end else begin
            delayed_a <= a;
        end
    end
    always @(negedge clk) begin
        if(reset || (delayed_a[96] | b[96])) begin
            o <= {1'b1,{96{1'b0}}};
            en = 1'b0;
        end else begin
            o <= {1'b0, out};
            en <= 1'b1;
        end
    end
endmodule