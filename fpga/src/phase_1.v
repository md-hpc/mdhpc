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
(* keep_hierarchy = "yes" *)
module phase_1 #(parameter N_CELL = 27)(
    input clk,
    input fast_clk,
    input reset,

    output CTL_DONE,
    input CTL_DOUBLE_BUFFER,
    input CTL_READY,
    output [(N_CELL*32)-1:0] oaddr,
    output [(N_CELL*32)-1:0] v_iaddr,
    input [(N_CELL*97)-1:0] r_v_caches,
    input [(N_CELL*97)-1:0] r_p_caches,
    output [32*N_CELL-1:0] iaddr,
    output [(N_CELL*97)-1:0] w_v_caches,
    output[(N_CELL)-1:0] wr_en
    );
    
    wire prc_done;
    
      (* keep = "true" *)reg  [113:0]p_ring_regs[N_CELL-1:0];
      (* keep = "true" *)wire  [113:0]p_ring_next[N_CELL-1:0];
    //wire [31:0] p_ring_addrs[N_CELL-1:0];
    
      (* keep = "true" *)wire [113:0]p_ring_reference[N_CELL-1:0];
    
    //output  neighbor,
    //output neighbor_cell,
     (* keep = "true" *)wire [(114)-1:0]p_ring_neighbor[N_CELL-1:0];
     (* keep = "true" *)wire [(8)- 1:0]p_ring_neighbor_cell[N_CELL-1:0];
    
     (* keep = "true" *)wire [113:0] pipeline_reference_out [N_CELL-1:0];
     (* keep = "true" *)wire [113:0] pipeline_neighbor_out [N_CELL-1:0];
    (* keep = "true" *) wire [N_CELL-1:0] pipeline_done;
    //reg [13:0] p_ring_addrs;
    
     (* keep = "true" *)reg [137:0] v_ring_regs [N_CELL-1:0];
   (* keep = "true" *) wire  [113:0]v_ring_next[N_CELL-1:0];
     (* keep = "true" *)wire  [32*3:0]fragment_out[N_CELL-1:0];
    //wire  [32:0] v_ring_addr[N_CELL-1:0];
    wire [N_CELL-1:0]v_rempty;
    
    reg finished_batch;
    reg finished_all;
    reg inflight;
    //wire finished_batch;
    //wire finished_all;
    
    wire [1:0] dispatch;
    
    wire [N_CELL*2:0] done_acc;
    
    wire [N_CELL-1:0] done_batch;
    wire [N_CELL-2:0] batch_acc;
    wire [N_CELL-1:0] done_all;
    wire [N_CELL-2:0] all_acc;
    wire [N_CELL-1:0] in_flight;
    wire [N_CELL-2:0] flight_acc;
    
    assign done_acc[0] = prc_done;
    assign CTL_DONE = done_acc[N_CELL*2];
    assign batch_acc[0] = done_batch[0] & done_batch[1];
    assign all_acc[0] = done_all[0] & done_all[1];
    assign flight_acc[0] = in_flight[0] | in_flight[1];
    
    integer iter;
    genvar i;
    generate
        for(i = 0; i < N_CELL; i=i+1) begin
        (* keep_hierarchy = "yes" *)
         ComputePipeline cp(.clk(clk),.fast_clk(fast_clk),.reset(reset),.reference(p_ring_reference[i]),.neighbor_cell(p_ring_neighbor_cell[i]),.neighbor(p_ring_neighbor[i]),.neighbor_out(pipeline_neighbor_out[i]),.reference_out(pipeline_reference_out[i]),.done(pipeline_done[i]),.read_controller_done(prc_done));
         VelocityRingNode vn(.clk(fast_clk),.reset(reset),.Cell(i),.neighbor_cell(pipeline_neighbor_out[i][106+:8]),.neighbor(pipeline_neighbor_out[i][105:0]),.reference(pipeline_reference_out[i][105:0]),.reference_cell(pipeline_reference_out[i][106+:8]),.next(v_ring_next[i][105:0]),.next_cell(v_ring_next[i][106+:8]),.prev(v_ring_next[(i+N_CELL-1)%N_CELL][105:0]),.prev_cell(v_ring_next[(i+N_CELL-1)%N_CELL][106+:8]),.fragment_out(fragment_out[i]),.rempty(v_rempty[i]),.addr(oaddr[32*i+:32]),.v_iaddr(v_iaddr[32*i+:32]));
         PositionRingNode pn(.clk(clk),.fast_clk(fast_clk),.reset(reset),.Cell(i),.dispatch(dispatch),.next(p_ring_next[i][105:0]),.next_cell(p_ring_next[i][106+:8]),.prev(p_ring_next[(i+N_CELL-1)%N_CELL][105:0]),.prev_cell(p_ring_next[(i+N_CELL-1)%N_CELL][106+:8]),.reference(p_ring_reference[i]),.neighbor(p_ring_neighbor[i]),.neighbor_cell(p_ring_neighbor_cell[i]),.double_buffer(CTL_DOUBLE_BUFFER),.done_batch(done_batch[i]),.done_all(done_all[i]),.in_flight(in_flight[i]),.bram_in(r_p_caches[i*97+:97]),.addr(iaddr[32*i+:32]));
         Adder adder(.clk(clk),.reset(reset),.a(fragment_out[i]),.b(r_v_caches[97*i+:97]),.en(wr_en[i]),.o(w_v_caches[97*i+:97]));
         assign done_acc[i+1] = done_acc[i] & pipeline_done[i];
         assign done_acc[i+N_CELL+1] = done_acc[i+N_CELL] & v_rempty[i];
        end
        for (i = 1; i < N_CELL - 1; i=i+1) begin
            assign batch_acc[i] = batch_acc[i-1] & done_batch[i+1];
            assign all_acc[i] = all_acc[i-1] & done_all[i+1];
            assign flight_acc[i] =flight_acc [i-1] | in_flight[i+1];
        end
    endgenerate
    PositionReadController prc(.ready(CTL_READY),.clk(clk),.reset(reset),.finished_batch(finished_batch),.finished_all(finished_all),.in_flight(inflight),.dispatch(dispatch),.done(prc_done));
    
    always @ (posedge clk) begin
    if(reset) begin
            for(iter = 0; iter<N_CELL; iter = iter + 1) begin
            p_ring_regs[iter] <= {114{1'b1}};
            v_ring_regs[iter] <= {161{1'b1}};
            finished_batch <= 0;
            finished_all <= 0;
            inflight <= 0;
        end
    end else begin    
        finished_batch <= batch_acc[N_CELL-2];
        finished_all <= all_acc[N_CELL-2];
        inflight <= flight_acc[N_CELL-2];
        for(iter = 0; iter<N_CELL; iter = iter + 1) begin
            p_ring_regs[iter] <= p_ring_next[iter];
            v_ring_regs[iter] <= v_ring_next[iter];
        
        end
    end
    end
endmodule
