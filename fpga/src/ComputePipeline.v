`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/17/2025 05:54:48 PM
// Design Name: 
// Module Name: ComputePipeline
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
module ComputePipeline(
    input read_controller_done,
    input clk,
    input fast_clk,
    input reset,
    input [113:0] reference,
    input [105:0] neighbor,
    input [7:0] neighbor_cell,
    output done,
    output [113:0] neighbor_out,
    output [113:0] reference_out
    //output  [96:0] reference,
    //output  [96:0] neighbor
    );
    
    integer iter;
    genvar i;
    reg [3:0] counter;
    (* keep = "true" *)wire  [226:0] filter_bank_out;
    (* keep = "true" *)wire  [226:0] pair_queue_out;
   (* keep = "true" *) wire  [226:0] force_pipeline_out;
    (* keep = "true" *)wire pq_empty;
    (* keep = "true" *)wire [113:0] _reference;
    (* keep = "true" *)reg [113:0] _reference_buff;
    
    wire _done;
    assign done = _done;
    reg null_pf;
    wire and_pf;
    //assign and_acc_pf[0] = filter_bank_out[192];
    assign and_pf = null_pf & filter_bank_out[226];
    (* keep_hierarchy = "yes" *)
    ParticleFilter pf(.fast_clk(fast_clk),.reset(reset),.neighbor(neighbor),.neighbor_cell(neighbor_cell),.reference({_reference_buff[113:105],_reference_buff[96:0]}),.reference_cell(_reference_buff[104:97]),.o(filter_bank_out));
    /*
    generate
        for(i = 0; i < 14; i=i+1) begin
         //ParticleFilter pf(.neighbor(neighbors[i*97+:97]),.neighbor_cell(neighbors[(i*105 + 97)+:8]),.reference(_reference[96:0]),.reference_cell(_reference[104:97]),.o(filter_bank_out[193*i+:193]));
        end
        for(i = 1; i < 14; i=i+1) begin
            assign and_acc_pf[i] = and_acc_pf[i-1] &  filter_bank_out[193*i+192];
        end
    endgenerate
    */
    PairQueue pq(.clk(fast_clk),.reset(reset),.in(filter_bank_out),.out(pair_queue_out),.qempty(pq_empty));
    ForcePipeline fp(.clk(clk),.reset(reset),.in(pair_queue_out),.out(force_pipeline_out),.done(force_pipeline_done));
    PipelineReader pr(.clk(clk),.reset(reset),.in(force_pipeline_out),.neighbor(neighbor_out),.reference(reference_out),.done(_done));
    Noop ref(.i(reference),.o(_reference));
    
    assign _done = read_controller_done & and_pf & pair_queue_out[226] & force_pipeline_done;
    
    
    always @(posedge fast_clk) begin
        if(reset) begin
            counter <= 4'd15;
            null_pf <= 1;
            _reference_buff <= reference;
        end else begin
            if(counter == 15) begin
                counter <= 0;
                null_pf <= 1;
                _reference_buff <= reference;
            end else begin
                null_pf <= and_pf;
                counter <= counter + 1;
            end
            
            
        end
    end
endmodule
