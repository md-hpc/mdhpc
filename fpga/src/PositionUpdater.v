`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/09/2025 03:24:34 PM
// Design Name: 
// Module Name: PositionUpdater
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

module PositionUpdater #(parameter DBSIZE = 256, parameter L = 10, parameter cutoff = 1, parameter UNIVERSE = 3)(
    input wire clk,
    input wire rst,
    input wire ready,
    input wire [1:0] double_buffer,
    input wire [32:0] overwrite_addr,
    input wire [(32*3):0] vi,
    input wire [(32*3):0] pi,
    input wire [(32*3):0] nodePosIn,
    input wire [(32*3):0] nodeVelIn,
    input wire [32:0] nodeCellIn,
    input wire [32:0] Cell,
    input wire stop_we,
    output reg [32:0] iaddr,
    output reg [(32*3):0] vo,
    output reg [(32*3):0] po,
    output reg we,
    output reg done,
    output [1:0] block,
    output  [(32*3):0] nodePOut,
    output  [(32*3):0] nodeVOut,
    output  [32:0] nodeCOut
);
    
    reg [(32*3):0] nodePosOut;
    reg [(32*3):0] nodeVelOut;
    reg [32:0] nodeCellOut;
    
    reg [(32*3):0] _nodePosOut;
    reg [(32*3):0] _nodeVelOut;
    reg [32:0] _nodeCellOut;
    
    reg rdy;
    wire [32:0] cIndex;
    wire [(32*3):0] newp;
    CellIndex CI(.pi(pi),.vi(vi),.cIndex(cIndex),.newp(newp));
    reg [32:0] new_addr;
    reg [(32*3):0] nodePos, nodeVel;
    reg [32:0] nodeCell;
    
    assign nodePOut = (nodePosOut != _nodePosOut)? nodePosOut : {1'b1,{96{1'b0}}};
    assign nodeVOut = (nodeVelOut != _nodeVelOut)? nodeVelOut : {1'b1,{96{1'b0}}};
    assign nodeCOut = (nodeCellOut != _nodeCellOut)? nodeCellOut : {1'b1,{32{1'b0}}};
    
    assign block = (rst || !rdy || overwrite_addr[32] != 1'b1)? 2'b11: 
                    (nodePos[96] != 1'b1 && nodeVel[96] != 1'b1 && nodeCell[32] != 1'b1)?2'b01:2'b00 ;
    always @(negedge clk) begin
        _nodePosOut <= nodePosOut;
        _nodeVelOut <= nodeVelOut;
        _nodeCellOut <= nodeCellOut;
        rdy <= ready;
        if (rst) begin
            new_addr <= {1'b1,{32{1'b0}}};
            nodePos <= {1'b1,{96{1'b0}}};
            nodeVel <= {1'b1,{96{1'b0}}};
            nodeCell <= {1'b1,{32{1'b0}}};
            done <= 0;
            //block <= 2'b11;
            iaddr <= {1'b1,{32{1'b0}}};
            vo <= {1'b1,{96{1'b0}}};
            po <= {1'b1,{96{1'b0}}};
            we <= 1'b0;
            nodePosOut <= {1'b1,{96{1'b0}}};
            nodeVelOut <= {1'b1,{96{1'b0}}};
            nodeCellOut <= {1'b1,{32{1'b0}}};
        end else if (!rdy) begin
            new_addr <= (double_buffer[0] == 1) ?  0 : DBSIZE;
            done <= 0;
            //block <= 2'b11;
            iaddr[32] <= 1'b0;
            iaddr[0+:32] <= (double_buffer[0] == 1) ?  0 : DBSIZE;
            vo <= {1'b1,{96{1'b0}}};
            po <= {1'b1,{96{1'b0}}};
            we <= 1'b0;
            nodePosOut <= {1'b1,{96{1'b0}}};
            nodeVelOut <= {1'b1,{96{1'b0}}};
            nodeCellOut <= {1'b1,{32{1'b0}}};
        end else if (overwrite_addr[32] != 1'b1) begin
            iaddr <= overwrite_addr;
            vo <= {1'b1,{96{1'b0}}};
            po <= {1'b1,{96{1'b0}}};
            we <= 1'b1;
            done <= 0;
            nodePosOut <= {1'b1,{96{1'b0}}};
            nodeVelOut <= {1'b1,{96{1'b0}}};
            nodeCellOut <= {1'b1,{32{1'b0}}};
            //block <= 2'b11;
        end else if (nodePos[96] != 1'b1 && nodeVel[96] != 1'b1 && nodeCell[32] != 1'b1) begin
            done <= 0;
            //block <= 2'b01;
            nodePos <= {1'b1,{96{1'b0}}};
            nodeVel <= {1'b1,{96{1'b0}}};
            nodeCell <= {1'b1,{32{1'b0}}};
            if (nodeCell != Cell) begin
                nodePosOut <= nodePos;
                nodeVelOut <= nodeVel;
                nodeCellOut <= nodeCell;
                 nodePos <= {1'b1,{96{1'b0}}};
                 nodeVel <= {1'b1,{96{1'b0}}};
                 nodeCell <= {1'b1,{32{1'b0}}};
                we <= 1'b0;
            end else begin
                nodePosOut <= {1'b1,{96{1'b0}}};
                nodeVelOut <= {1'b1,{96{1'b0}}};
                nodeCellOut <= {1'b1,{32{1'b0}}};
                po <= nodePos;
                vo <= nodeVel;
                we <= 1'b1;
            end
        end else begin
            nodePos <= nodePosIn;
            nodeVel <= nodeVelIn;
            nodeCell <= nodeCellIn;

            if (pi[96] == 1'b1 && vi[96] == 1'b1  && nodePosIn[96] == 1'b1  && nodeVelIn[96] == 1'b1  && nodeCellIn[32] == 1'b1 ) begin
                done <= 1;
                //iaddr <= {1'b1,{32{1'b0}}};
                po <= {1'b1,{96{1'b0}}};
                vo <= {1'b1,{96{1'b0}}};
                we <= 1'b0;
                //block <= 2'b00;
                nodePosOut <= {1'b1,{96{1'b0}}};
                nodeVelOut <= {1'b1,{96{1'b0}}};
                nodeCellOut <= {1'b1,{32{1'b0}}};
            end else begin
                done <= 0;
                //nodeCellOut <= (pi[8:0]/CUTOFF)%UNIVERSE_SIZE;
                // Assume vi * DT = vi, since DT = 1
                // NEWCELL = ((pi[7:0]+ (vi * DT)) % L)/CUTOFF)%UNIVERSE_SIZE) + ((pi[15:8]+ (vi * DT)) % L)/CUTOFF)%UNIVERSE_SIZE)*UNIVERSE_SIZE + ((pi[23:16]+ (vi * DT)) % L)/CUTOFF)%UNIVERSE_SIZE) * UNIVERSE_SIZE * UNIVERSE_SIZE
                // NEWCELL = ((pi[7:0]+ vi) % L)/CUTOFF)%UNIVERSE_SIZE) + ((pi[15:8]+ vi) % L)/CUTOFF)%UNIVERSE_SIZE)*UNIVERSE_SIZE + ((pi[23:16]+ vi) % L)/CUTOFF)%UNIVERSE_SIZE) * UNIVERSE_SIZE * UNIVERSE_SIZE
                iaddr <= new_addr;
                new_addr <= new_addr + 1;
                
                //Cell == NEWCELL
                if(Cell == cIndex) begin
                    po <= newp;
                    vo <= vi;
                    we <= 1'b1;
                    
                    iaddr[32] = 1'b0;
                    //block <=2'b00;
                    nodePosOut <= {1'b1,{96{1'b0}}};
                    nodeVelOut <= {1'b1,{96{1'b0}}};
                    nodeCellOut <= {1'b1,{32{1'b0}}};
                end else begin
                    //block <= 2'b00;
                    nodePosOut <= newp;
                    nodeVelOut <= vi;
                    nodeCellOut <= cIndex;
                    po <= {1'b1,{96{1'b0}}};
                    vo <= {1'b1,{96{1'b0}}};
                    iaddr <= {1'b1,{32{1'b0}}};
                    we <= 1'b0;
                
                
                end
            end
        end
    end

endmodule
