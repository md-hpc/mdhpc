`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 07:57:53 PM
// Design Name: 
// Module Name: ControlUnit
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
module ControlUnit(
    input clk,
    input reset,
    input phase1_done,
    input phase3_done,
    input mem_set,
    output phase1_ready,
    output phase3_ready,
    output double_buffer,
    input [31:0] step
    );
    reg [1:0] phase;
    reg double_buff;
    assign double_buffer = double_buff;
    assign phase1_ready = phase == 0;
    assign phase3_ready = phase == 1;
    // PHASE 1 -> phase = 0
    // PHASE 3 -> phase = 1
    reg [31:0] step_counter;
    always @(posedge clk, posedge reset) begin
        if(reset) begin
            phase <= 2;
            double_buff <= 0;
            step_counter <= 0;
        end else begin
            if(mem_set && phase == 2) begin
                phase <= 0; //Change this back to 0!!!!!
            end else if(mem_set) begin
                if(phase == 0 && phase1_done == 1) begin
                    phase <= 1;
                end else if (phase == 1 && phase3_done == 1 && step_counter != step) begin
                    phase <= 0;
                    step_counter <= step;
                    double_buff <= ~double_buff;
                end
            end
            
        end
    end
endmodule


/*

        if self.phase == PHASE1 and self.phase1_done.get():
            self.phase = PHASE3
        elif self.phase == PHASE3 and self.phase3_done.get():
            self.phase = PHASE1
            self._double_buffer = 0 if self._double_buffer else 1
            self.t += 1

        self.double_buffer.set(self._double_buffer)
        self.phase1_ready.set(self.phase == PHASE1)
        self.phase3_ready.set(self.phase == PHASE3)
        
        */