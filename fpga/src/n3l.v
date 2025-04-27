`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 03:27:23 PM
// Design Name: 
// Module Name: n3l
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


module n3l(
    input [95:0] reference,
    input [95:0] neighbor,
    output o,
    output mo
    );
    wire [96:0] mod;
    modr m(reference,neighbor,mod);
    assign mo = mod;
    assign o = (mod[0+:32] != {32{1'b0}}) ?  (mod[31] == 1 && mod[0+:31] > 0)? 0 :  1 :(mod[32+:32] != {32{1'b0}}) ?  (mod[63] == 1 && mod[32+:31] > 0)? 0 :  1 : (mod[64+:32] != {32{1'b0}}) ?  (mod[95] == 1 && mod[64+:31] > 0)? 0 :  1 : 0;
    
endmodule
