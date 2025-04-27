`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 02:18:12 PM
// Design Name: 
// Module Name: modd
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


module modd(
    input [31:0] a,
    input [31:0] b,
    input [31:0] M,
    output [31:0] o
    );
    //opts = [(b-M)-a, b-a, (b+M)-a]
    //which is equal to
    //opts = [(b-a)-M, (b-a), (b-a)+M]
    wire [31:0] opta;
    wire [31:0] opta_im;
    wire [31:0] optb;
    wire [31:0] optc;
    wire [31:0] optc_im;
    
    wire [31:0] comp_im;
    
    fp32_sub sub_b(b,a,optb);
    
    fp32_sub sub_ima(optb,M,opta);
    
    fp32_add add_imc(optb,M,1'b0,optc);
    
    fp32_abs_comp comp_a(opta,optb,comp_im);
    fp32_abs_comp comp_b(comp_im,optc,o);
    
    
endmodule
