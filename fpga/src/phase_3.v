`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/09/2025 01:54:20 PM
// Design Name: 
// Module Name: phase_1
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


module phase_3 #(parameter N_CELL = 27)(
    input clk,
    input reset,
    input CTL_READY,
    input CTL_DOUBLE_BUFFER,
    output [N_CELL*2-1:0] CTL_DONE,
    output [(N_CELL*32)-1:0] oaddr,
    input [(N_CELL*97)-1:0] r_v_caches,
    input [(N_CELL*97)-1:0] r_p_caches,
    output [32*N_CELL-1:0] iaddr,
    output [(N_CELL*97)-1:0] w_v_caches,
    output [(N_CELL*97)-1:0] w_p_caches,
    output[(N_CELL)-1:0] wr_en
    
    );
    wire [N_CELL-1:0] stop_we;
    wire [1:0] block [N_CELL-1:0];
    wire [32:0] overwrite_addr[N_CELL-1:0];
    wire [(32*3):0] ring_pos_reg [N_CELL-1:0];
    wire [(32*3):0] ring_vel_reg [N_CELL-1:0];
    wire [(32):0] ring_cell_reg [N_CELL-1:0];
    genvar i;
generate
    for (i = 0; i < N_CELL; i = i + 1) begin 
    
    
        PositionUpdateController puc (
            .clk(clk),
            .rst(reset),
            .ready(CTL_READY),
            .double_buffer(CTL_DOUBLE_BUFFER),
            .block(block[i]),
            .done(CTL_DONE[i]),
            .oaddr(oaddr[i*32+:32]),
            .overwrite_addr(overwrite_addr[i]),
            .stop_we(stop_we[i])
        );

        PositionUpdater pu (
            .clk(clk),
            .rst(reset),
            .ready(CTL_READY),
            .double_buffer(CTL_DOUBLE_BUFFER),
            .overwrite_addr(overwrite_addr[i]),
            .vi(r_v_caches[i*97+:97]),
            .pi(r_p_caches[i*97+:97]),
            .nodePosIn(ring_pos_reg[i]),
            .nodeVelIn(ring_vel_reg[i]),
            .nodeCellIn(ring_cell_reg[i]),
            .iaddr(iaddr[i*32+:32]),
            .vo(w_v_caches[i*97+:97]),
            .po(w_p_caches[i*97+:97]),
            .done(CTL_DONE[i+N_CELL]),
            .block(block[i]),
            .nodePOut(ring_pos_reg[(i+1)%N_CELL]),
            .nodeVOut(ring_vel_reg[(i+1)%N_CELL]),
            .nodeCOut(ring_cell_reg[(i+1)%N_CELL]),
            .Cell(i),
            .we(wr_en[i]),
            .stop_we(stop_we[i])
        );
    end
endgenerate
endmodule
