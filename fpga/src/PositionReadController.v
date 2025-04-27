`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/10/2025 09:13:45 PM
// Design Name: 
// Module Name: PositionReadController
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
module PositionReadController (
    input clk,
    input reset,
    input ready,
    input finished_batch,
    input finished_all,
    input in_flight,
    output reg [1:0] dispatch,
    output reg [1:0] done
);

    reg [1:0] startup;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            startup <= 0;
            dispatch <= 2'b11;
            done <= 0;
        end else if (!ready) begin
            startup <= 0;
            dispatch <= 2'b11;
            done <= 0;
        end else begin
            case (startup)
                0: begin
                    dispatch <= 1;
                    done <= 0;
                    startup <= 1;
                end
                1: begin
                    if (!in_flight) begin
                        dispatch <= 1;
                        done <= 0;
                        startup <= 2;
                    end else begin
                        dispatch <= 1;
                        done <= 0;
                    end
                end
                default: begin
                    if (finished_batch && finished_all) begin
                        dispatch <= 0;
                        done <= 1;
                    end else if (finished_batch && !in_flight) begin
                        dispatch <= 1;
                        done <= 0;
                    end else begin
                        dispatch <= 0;
                        done <= 0;
                    end
                end
            endcase
        end
    end

endmodule

