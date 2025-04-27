`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 02:24:03 PM
// Design Name: 
// Module Name: fp32_abs_comp
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


module fp32_abs_comp (
    input [31:0] a,  
    input [31:0] b,
    output [31:0] min
);

    wire [30:0] abs_a = {1'b0, a[30:0]};
    wire [30:0] abs_b = {1'b0, b[30:0]};
    
    
    assign min = (abs_a <= abs_b)? a: b;

endmodule

