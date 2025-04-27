`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/12/2025 11:24:15 AM
// Design Name: 
// Module Name: fp32_mul
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


module fp32_mul (
    input wire [31:0] a,  // FP32 input A
    input wire [31:0] b,  // FP32 input B
    output  [31:0] o   // FP32 output (A * B)
);

 fp_mult mult(.s_axis_a_tdata(a),.s_axis_a_tvalid(1),.s_axis_b_tvalid(1),.s_axis_b_tdata(b),.m_axis_result_tdata(o));

    
endmodule

