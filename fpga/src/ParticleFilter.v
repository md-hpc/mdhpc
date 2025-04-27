`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 03:24:17 PM
// Design Name: 
// Module Name: ParticleFilter
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

(* keep_hierarchy = "yes" *)
module ParticleFilter(
    input fast_clk,
    input reset,
    input [105:0] reference,
    input [105:0] neighbor,
    input [7:0] reference_cell,
    input [7:0] neighbor_cell,
    output [226:0] o
    );
    
     (* keep = "true" *) wire n3l_w;
    (* keep = "true" *)  wire[95:0] mod;
     (* keep = "true" *) wire[31:0] r;
    
     (* keep = "true" *) wire less;
    
     (* keep = "true" *) reg [226:0] im_o;
    assign o = im_o;
    
    n3l n3l_module(reference[0+:96],neighbor[0+:96],n3l_w,mod);
    
    //modr m(reference[0+:96],neighbor[0+:96],mod);
    Norm norm(mod,r);
    
    
    fp32_lessthan lessthan(r,32'h40c80000,less);
    
    always @(negedge fast_clk, posedge reset) begin
        if(reset) begin
            im_o <= {1'b1,{34{1'b0}},{192{1'b0}}};
        end else begin
            im_o <= (reference[96] == 1 || neighbor[96] == 1 || (neighbor_cell == reference_cell && ~n3l_w) || (reference[97+:9] == neighbor[97+:9] && neighbor_cell == reference_cell) || ~less )?{1'b1,{34{1'b0}},{192{1'b0}}} : {1'b0,reference_cell,reference[97+:9],reference[0+:96], neighbor_cell,neighbor[97+:9],neighbor[0+:96]};
        end
    
    end
endmodule
