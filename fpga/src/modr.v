`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 02:29:20 PM
// Design Name: 
// Module Name: modr
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


module modr(
    input [95:0] reference,
    input [95:0] neighbor,
    output [95:0] r
    );
    
    modd modd_a(reference[0+:32],neighbor[0+:32],32'h40F00000,r[0+:32]);
    modd modd_b(reference[32+:32],neighbor[32+:32],32'h40F00000,r[32+:32]);
    modd modd_c(reference[64+:32],neighbor[64+:32],32'h40F00000,r[64+:32]);
    
endmodule
