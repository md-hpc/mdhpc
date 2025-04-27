`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 01:22:11 PM
// Design Name: 
// Module Name: ForcePipeline
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


module ForcePipeline(
    input clk,
    input reset,
    input [226:0] in, //two velocities are in the input:
    output done,
    output reg [226:0] out
    //output reg [96:0] reference,
    //output reg [96:0] neighbor
    );
    
    wire [95:0] lj_force;
    LJ lj(in[113+:96],in[0+:96],lj_force);
    assign done = in[226];
    always @(negedge clk, posedge reset) begin
        if(reset || in[226] == 1) begin
            out <= {1'b1,{226{1'b0}}};
        end else begin
            out[0+:96] <= lj_force;
            out[96+:17] <= in[209+:17];
            out[113+:31] <= lj_force[0+:31];
            out[144] <= ~lj_force[31];
            out[145+:31] <= lj_force[32+:31];
            out[176] <= ~lj_force[63];
            
            out[177+:31] <= lj_force[64+:31];
            out[208] <= ~lj_force[95];
            out[209+:17] <= in[96+:17];
            out[226] <= 0;
    
        end
    end
    
    
    
    
endmodule
