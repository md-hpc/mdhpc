`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/22/2025 04:08:54 PM
// Design Name: 
// Module Name: n3l_cell
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


module n3l_cell #(parameter UNIVERSE_SIZE = 3)(
    input [31:0] reference,
    input [31:0] neighbor,
    output o
    );
    wire [31:0] cubic_reference_x;
    wire [31:0] cubic_reference_y;
    wire [31:0] cubic_reference_z;
    wire [31:0] cubic_neighbor_x;
    wire [31:0] cubic_neighbor_y;
    wire [31:0] cubic_neighbor_z;
    
    wire [32:0] modd_x;
    wire [32:0] modd_y;
    wire [32:0] modd_z;
    wire [31:0] modd_x_abs;
    wire [31:0] modd_y_abs;
    wire [31:0] modd_z_abs;
    
    
    assign cubic_reference_x = reference%UNIVERSE_SIZE;
    assign cubic_reference_y = (reference/UNIVERSE_SIZE)%UNIVERSE_SIZE;
    assign cubic_reference_z = ((reference/UNIVERSE_SIZE)/UNIVERSE_SIZE)%UNIVERSE_SIZE;
    
    assign cubic_neighbor_x = neighbor%UNIVERSE_SIZE;
    assign cubic_neighbor_y = (neighbor/UNIVERSE_SIZE)%UNIVERSE_SIZE;
    assign cubic_neighbor_z = ((neighbor/UNIVERSE_SIZE)/UNIVERSE_SIZE)%UNIVERSE_SIZE;
    
    
    modd_cell modd_a(cubic_reference_x,cubic_neighbor_x,UNIVERSE_SIZE,modd_x,modd_x_abs);
    modd_cell modd_b(cubic_reference_y,cubic_neighbor_y,UNIVERSE_SIZE,modd_y,modd_y_abs);
    modd_cell modd_c(cubic_reference_z,cubic_neighbor_z,UNIVERSE_SIZE,modd_z,modd_z_abs);
    
    
    assign o = (modd_x_abs > 1 | modd_y_abs > 1 | modd_z_abs > 1)? 1'b0:
               ((modd_x[32] == 1'b1 && modd_x_abs > 0) || (modd_x[32] == 1'b0 && modd_x_abs > 1))? 1'b0:
               (modd_x[32]== 1'b0 && modd_x > 0 )? 1'b1:
               ((modd_y[32] == 1'b1 && modd_y_abs > 0) || (modd_y[32] == 1'b0 && modd_y_abs > 1))? 1'b0:
               (modd_y[32]== 1'b0 && modd_y > 0 )? 1'b1:
               ((modd_z[32] == 1'b1 && modd_z_abs > 0) || (modd_z[32] == 1'b0 && modd_z_abs > 1))? 1'b0:
               (modd_z[32]== 1'b0 && modd_z > 0 )? 1'b1: 1'b1;
               
    
    
endmodule
