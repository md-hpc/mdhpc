`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 01:50:51 PM
// Design Name: 
// Module Name: Norm
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


module Norm(
    input [95:0] in,
    output [31:0] out
    );
    
    wire [31:0] aa;
    wire [31:0] bb;
    wire [31:0] cc;
    
    wire[31:0] square_dist_a;
    wire[31:0] square_dist;
    
    
    fp32_mul mult_a(in[0+:32],in[0+:32],aa);
    fp32_mul mult_b(in[32+:32],in[32+:32],bb);
    fp32_mul mult_c(in[64+:32],in[64+:32],cc);
    
    fp32_add add_a(aa,bb,1'b0,square_dist_a);
    fp32_add add(square_dist_a,cc,1'b0,out);
    
    
endmodule
