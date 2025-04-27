`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 02:35:27 PM
// Design Name: 
// Module Name: LJ
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


module LJ #(parameter LJ_MAX = 32'h3820d27c, parameter CUTOFF = 32'h40200000, parameter UNIVERSE_SIZE = 32'h40400000, parameter DT = 32'h33d6bf95)(
    input [95:0] reference,
    input [95:0] neighbor,
    output [95:0] lj_out
    );
    
    wire[95:0] mod;
    wire[31:0] r;
    
    wire[31:0] r2;
    wire[31:0] r4;
    wire[31:0] r8;
    wire[31:0] r12;
    wire[31:0] r14;
    
    wire[31:0] rec_a;
    wire[31:0] rec_b;
    wire[31:0] rec;
    wire[31:0] coeff;
    wire[95:0] dist;
    
    wire [95:0] lj;
    wire [95:0] min_lj;
    wire [95:0] true_lj;
    modr m(reference,neighbor,mod);
    Norm norm(mod,r2);
    
    // LJ Computation: LJ = (160*(6.0/r**8.0-12.0/r**14.0)*(neighbor - reference))
    //fp32_mul mult1 (r,r,r2);
    fp32_mul mult2 (r2,r2,r4);
    fp32_mul mult3 (r4,r4,r8);
    fp32_mul mult4 (r8,r4,r12);
    fp32_mul mult5 (r12,r2,r14);
    
    fp32_div div_a(32'h38c9539c,r8,rec_a);
    fp32_div div_b(32'h3949539c,r14,rec_b);
    
    fp32_sub sub_rec(rec_a,rec_b,rec);
    
    fp32_sub sub_a(neighbor[0+:32],reference[0+:32],dist[0+:32]);
    fp32_sub sub_b(neighbor[32+:32],reference[32+:32],dist[32+:32]);
    fp32_sub sub_c(neighbor[64+:32],reference[64+:32],dist[64+:32]);
    
    fp32_mul mul_a(dist[0+:32],rec,lj[0+:32]);
    fp32_mul mul_b(dist[32+:32],rec,lj[32+:32]);
    fp32_mul mul_c(dist[64+:32],rec,lj[64+:32]);
    
    fp32_abs_comp_lj comp_a(lj[0+:32],LJ_MAX,min_lj[0+:32]);
    fp32_abs_comp_lj comp_b(lj[32+:32],LJ_MAX,min_lj[32+:32]);
    fp32_abs_comp_lj comp_c(lj[64+:32],LJ_MAX,min_lj[64+:32]);
    
    
    assign lj_out = r2 == 0? {95{1'b0}} : min_lj; 
    
endmodule
