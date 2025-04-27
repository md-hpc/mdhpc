`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/12/2025 11:13:34 AM
// Design Name: 
// Module Name: fp32_add
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


module fp32_add (
    input  [31:0] a,  // FP32 input A
    input  [31:0] b,  // FP32 input B
    input sub,
    output  [31:0] o   // FP32 output (A + B)
);
wire out_valid;

addsub add (.s_axis_a_tdata(a),.s_axis_a_tvalid(1),.s_axis_b_tvalid(1),.s_axis_b_tdata({b[31]^sub,b[0+:31]}),.m_axis_result_tdata(o),.m_axis_result_tvalid(out_valid));


endmodule