`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/09/2025 02:51:50 PM
// Design Name: 
// Module Name: PositionUpdateController
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


module PositionUpdateController #(parameter DBSIZE = 256)(
    input ready,
    output reg done,
    input double_buffer,
    input [1:0] block,
    output [31:0] oaddr,
    output reg [32:0] overwrite_addr,
    input clk,
    input rst,
    output reg stop_we
    );
    
    reg [31:0] raddr;
    reg [31:0] _raddr;
    reg [32:0] _overwrite_addr;
    
    
    wire [31:0] out_wire = (rst)? 0:
                            (!ready)? (double_buffer == 1) ? DBSIZE : 0 :_raddr;
    assign oaddr = out_wire;
    
    always @(posedge clk) begin
        stop_we <= overwrite_addr[32];
        if(rst) begin
            raddr <= 0;
            _raddr <= 0;
            _overwrite_addr <= {1'b1,{32{1'b0}}};
            done <= 0;
            //oaddr <= {32{1'b1}};
            overwrite_addr <={1'b1,{32{1'b0}}};
        end else  if(!ready) begin
            raddr <= (double_buffer == 1) ? DBSIZE : 0;
            _raddr <= (double_buffer == 1) ? DBSIZE : 0;
            _overwrite_addr[32] <= 0;
            _overwrite_addr[0+:32] <= (double_buffer == 1) ? 0 : DBSIZE;
            done <= 0;
            //oaddr <= {32{1'b1}};
            overwrite_addr <= {1'b1,{32{1'b0}}};
        end else  if(_raddr[0+:32] == ((double_buffer == 1) ? DBSIZE : 0) + DBSIZE - 1) begin
            done <= 1;
            //raddr <= 0;
            //oaddr <= {32{1'b1}};
            _overwrite_addr <= {1'b1,{32{1'b0}}};
            overwrite_addr <= {1'b1,{32{1'b0}}};
        end else  begin
            //oaddr <= raddr;
            overwrite_addr <= _overwrite_addr;
            done <= 0;
            raddr <= _raddr;
            if(_overwrite_addr[32] == 1'b1 && block != 2'b01) begin
                _raddr <= _raddr + 1;
            end else begin
            
                
                if (_overwrite_addr[0+:32] == ((double_buffer == 1) ? 0 : DBSIZE) + DBSIZE - 1) begin
                    _overwrite_addr <= {1'b1,{32{1'b0}}};
                    _raddr <= (double_buffer == 1) ? DBSIZE : 0;
                 end else if(_overwrite_addr[32] != 1)begin
                    _overwrite_addr <= _overwrite_addr + 1;
                 
                 end
            end
        end
    end
    
    
endmodule
