`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/12/2025 11:22:34 AM
// Design Name: 
// Module Name: fp32_mod
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


module fp32_mod (
    input wire [31:0] a,  // FP32 input A (Dividend)
    input wire [31:0] b,  // FP32 input B (Divisor)
    output wire [31:0] o  // FP32 output (Remainder)
);

    wire [31:0] div_result;
    wire [31:0] floor_div;
    wire [31:0] mult_result;
    wire [31:0] sub_result;

    // FP32 Division: a / b
    fp32_div div_unit (
        .a(a),
        .b(b),
        .o(div_result)
    );

    // Truncate division result to integer (floor)
    fp32_floor floor_unit (
        .a(div_result),
        .o(floor_div)
    );

    // Multiply: b * floor(a / b)
    fp32_mul mult_unit (
        .a(b),
        .b(floor_div),
        .o(mult_result)
    );

    // Subtract: a - (b * floor(a / b))
    fp32_sub sub_unit (
        .a(a),
        .b(mult_result),
        .o(sub_result)
    );

    // Handle Special Cases: NaN for division by zero
    assign o = (b == 32'b0) ? 32'h7FC00000 : sub_result; // Return NaN if b == 0

endmodule

