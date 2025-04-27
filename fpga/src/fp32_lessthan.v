`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 03:43:39 PM
// Design Name: 
// Module Name: fp32_lessthan
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


module fp32_lessthan(
    input [31:0] a,
    input [31:0] b,
    output o
    );
    
    wire sign_a, sign_b;
    wire [7:0] exp_a, exp_b;
    wire [22:0] mant_a, mant_b;
    wire a_less_b;
    
    // Extract sign, exponent, and mantissa
    assign sign_a = a[31];
    assign sign_b = b[31];
    assign exp_a = a[30:23];
    assign exp_b = b[30:23];
    assign mant_a = a[22:0];
    assign mant_b = b[22:0];
    
    // Comparison logic
    assign a_less_b = (sign_a & ~sign_b) ? 1'b1 :          // Negative vs Positive
                      (~sign_a & sign_b) ? 1'b0 :          // Positive vs Negative
                      (sign_a & sign_b) ?                  // Both negative
                          ((exp_a > exp_b) ? 1'b1 : (exp_a < exp_b) ? 1'b0 : (mant_a > mant_b)) :
                          ((exp_a < exp_b) ? 1'b1 : (exp_a > exp_b) ? 1'b0 : (mant_a < mant_b));
    
    // Output assignment
    assign o = a_less_b ? 1 : 0;
    
endmodule
