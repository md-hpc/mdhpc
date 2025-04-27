`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/12/2025 11:27:22 AM
// Design Name: 
// Module Name: fp32_sub
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


module fp32_sub (
    input wire [31:0] a,  // FP32 input A
    input wire [31:0] b,  // FP32 input B
    output wire [31:0] o  // FP32 output (A - B)
);


    // Instantiate FP32 Adder (Assuming it's named 'fp32_add')
    fp32_add adder (
        .a(a),
        .b(b),
        .o(o),
        .sub(1)
    );

endmodule