`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/15/2025 05:13:10 PM
// Design Name: 
// Module Name: PipelineReader
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


module PipelineReader(
    input clk,
    input reset,
    input [226:0] in, //two velocities are in the input:
    input done,
    output reg [113:0] reference,
    output reg [113:0] neighbor
    );
    
    reg [113:0] ref;
    reg [113:0] neigh;
    
    wire [31:0] ref_add [2:0];
    wire [31:0] neigh_add [2:0];
    
    fp32_add add_ref_x(.a(ref[0+:32]),.b(in[113+:32]),.o(ref_add[0]),.sub(0));
    fp32_add add_ref_y(.a(ref[32+:32]),.b(in[145+:32]),.o(ref_add[1]),.sub(0));
    fp32_add add_ref_z(.a(ref[64+:32]),.b(in[177+:32]),.o(ref_add[2]),.sub(0));
    
    fp32_add add_neigh_x(.a(neigh[0+:32]),.b(in[0+:32]),.o(neigh_add[0]),.sub(0));
    fp32_add add_neigh_y(.a(neigh[32+:32]),.b(in[32+:32]),.o(neigh_add[1]),.sub(0));
    fp32_add add_neigh_z(.a(neigh[64+:32]),.b(in[64+:32]),.o(neigh_add[2]),.sub(0));
    
    always @(posedge clk or posedge reset) begin
        if(reset) begin
            ref <= {{17{1'b0}},1'b1,{96{1'b0}}};
            neigh <= {{17{1'b0}},1'b1,{96{1'b0}}};
            
            reference <= {{17{1'b0}},1'b1,{96{1'b0}}};
            neighbor <= {{17{1'b0}},1'b1,{96{1'b0}}};
        end else begin
            if(done) begin
                if(ref[96] != 1) begin
                    reference <= ref;
                    ref <= {{17{1'b0}},1'b1,{96{1'b0}}};
                end else begin
                    reference <= {{17{1'b0}},1'b1,{96{1'b0}}};
                end
            
                if(neigh[96] != 1) begin
                    neighbor <= neigh;
                    neigh <= {{17{1'b0}},1'b1,{96{1'b0}}};
                end else begin
                    neighbor <= {{17{1'b0}},1'b1,{96{1'b0}}};
                end
            end  else if(in[226] != 1) begin
                if(ref[96] == 1) begin
                    ref <= {in[96+:17],in[226],in[0+:96]};
                    reference <= {{17{1'b0}},1'b1,{96{1'b0}}};
                end else if (ref[97+:17] != in[96+:17]) begin
                    reference <= ref;
                    ref <= {in[96+:17],in[226],in[0+:96]};
                end else begin
                    ref[95:0] <= {ref_add[2],ref_add[1],ref_add[0]};
                    reference <= {{17{1'b0}},1'b1,{96{1'b0}}};
                end
            
                if(neigh[96] == 1) begin
                    // {in[209+:17],in[226],in[113+:96]};
                    // {in[96+:17],in[226],in[0+:96]};
                    neigh <= {in[209+:17],in[226],in[113+:96]};
                    neighbor <= {{17{1'b0}},1'b1,{96{1'b0}}};
                end else if (neigh[97+:17] != in[209+:17]) begin
                    neighbor <= neigh;
                    neigh <= {in[209+:17],in[226],in[113+:96]};
                end else begin
                    neigh[95:0] <= {neigh_add[2],neigh_add[1],neigh_add[0]};
                    neighbor<= {{17{1'b0}},1'b1,{96{1'b0}}};
                end
            
            end else begin
                neighbor <= {{17{1'b0}},1'b1,{96{1'b0}}};
                reference <= {{17{1'b0}},1'b1,{96{1'b0}}};
            end
                
            
        end
    end
    
    
    
endmodule
