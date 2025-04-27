`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/22/2025 04:11:54 PM
// Design Name: 
// Module Name: modd_cell
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


module modd_cell(
    input [31:0] a,
    input [31:0] b,
    input [31:0] M,
    output [32:0] o,
    output [31:0] o_abs
    );
    //opts = [(b-M)-a, b-a, (b+M)-a]
    // Internal signed wires to hold the options

    // Pre-extended signed inputs
    wire signed [32:0] a_s =  $signed(a);
    wire signed [32:0] b_s = $signed(b);
    wire signed [32:0] M_s =  $signed(M);

    // Option candidates
    wire signed [32:0] opt1 = b_s - a_s;
    wire signed [32:0] opt0 = opt1 - M_s;
    wire signed [32:0] opt2 = opt1 + M_s;

    // Absolute values of options
    wire [32:0] abs0 = (opt0 < 0) ? -opt0 : opt0;
    wire [32:0] abs1 = (opt1 < 0) ? -opt1 : opt1;
    wire [32:0] abs2 = (opt2 < 0) ? -opt2 : opt2;

    // Minimum selector (one-hot encoding of selection)
    wire sel0 = (abs0 <= abs1) && (abs0 <= abs2);
    wire sel1 = (abs1 < abs0) && (abs1 <= abs2);
    wire sel2 = (abs2 < abs0) && (abs2 < abs1);

    // Final result based on selection
    assign o = sel0 ? opt0 :
                    sel1 ? opt1 :
                    opt2;
    assign o_abs = sel0 ? abs0[0+:32] :
                    sel1 ? abs1[0+:32] :
                    abs2[0+:32];
endmodule

