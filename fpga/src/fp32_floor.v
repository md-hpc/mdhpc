`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/12/2025 11:25:51 AM
// Design Name: 
// Module Name: fp32_floor
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


module fp32_floor (
    input wire [31:0] a,  // FP32 input
    output [31:0] o   // FP32 output (floor(a))
);

    // Extract IEEE-754 fields
    wire sign = a[31];
    wire [7:0] exp = a[30:23];
    wire [22:0] mantissa = a[22:0];

    // Compute integer threshold
    wire [7:0] shift_amount = 150 - exp;  // Number of fraction bits
    wire is_integer = (exp >= 150);       // No fraction part if exponent is large

    // Create a mask to zero out fractional bits
    wire [22:0] mask = (is_integer || exp < 127) ? 23'b0 : (23'b111111111111111111111111 << shift_amount);
    wire [22:0] truncated_mantissa = mantissa & mask;

    // Compute the floored result
    reg [31:0] floor_result;
    always @(*) begin
        if (exp < 127) begin
            // abs(x) < 1: floor(x) is 0 (or -1 if negative)
            floor_result = sign ? 32'hBF800000 : 32'b0;  // -1.0 or 0.0
        end else if (is_integer) begin
            // Already an integer, return original
            floor_result = a;
        end else begin
            // Truncate fractional part
            floor_result = {sign, exp, truncated_mantissa};
            
            // If negative and had a fraction, subtract 1
            if (sign && mantissa != truncated_mantissa)
                floor_result = floor_result - 32'h3F800000; // Subtract 1.0
        end
    end

    assign o = floor_result;

endmodule
