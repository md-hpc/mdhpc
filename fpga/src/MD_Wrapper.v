`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/20/2025 01:57:39 PM
// Design Name: 
// Module Name: MD_Wrapper
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


module MD_Wrapper  #(
      // Width of S_AXIS_n2k and M_AXIS_k2n interfaces
      parameter integer AXIS_TDATA_WIDTH      = 512,
      // Width of M_AXIS_summary interface
      parameter integer AXIS_SUMMARY_WIDTH    = 128,
      // Width of TDEST address bus
      parameter integer STREAMING_TDEST_WIDTH =  16,
      // Width of S_AXIL data bus
      parameter integer AXIL_DATA_WIDTH       = 32,
      // Width of S_AXIL address bus
      parameter integer AXIL_ADDR_WIDTH       =  9,
      
       parameter integer INIT_STEP_WIDTH       =   4,
       parameter integer N_CELL = 27
)(
    input  wire                                 ap_clk,
    input  wire                                 ap_rst_n,

    // AXI4-Stream host to streaming kernel 
    input  wire       [AXIS_TDATA_WIDTH-1:0]    S_AXIS_h2k_tdata,
    input  wire     [AXIS_TDATA_WIDTH/8-1:0]    S_AXIS_h2k_tkeep,
    input  wire                                 S_AXIS_h2k_tvalid,
    input  wire                                 S_AXIS_h2k_tlast,
    input  wire  [STREAMING_TDEST_WIDTH-1:0]    S_AXIS_h2k_tdest,
    output wire                                 S_AXIS_h2k_tready,
    // AXI4-Stream streaming kernel to host
    output wire      [AXIS_TDATA_WIDTH-1:0]    M_AXIS_k2h_tdata,
    output wire     [AXIS_TDATA_WIDTH/8-1:0]    M_AXIS_k2h_tkeep,
    output wire                                 M_AXIS_k2h_tvalid,
    output wire                                 M_AXIS_k2h_tlast,
    output wire  [STREAMING_TDEST_WIDTH-1:0]    M_AXIS_k2h_tdest,
    input  wire                                 M_AXIS_k2h_tready,
    
    input  wire                        [2:0]    MD_state_w,
    input  wire  [STREAMING_TDEST_WIDTH-1:0]    init_id_w,
    output [9:0] initcounter,
    output elem_read,
    input read_ctrl,
    input [31:0] step,
    output done,
    input [209:0] d_in,
    output [191:0] d_out,
    input elem_write,
    output [31:0] out_count,
    output [96:0] out_pos
    );
    
    reg [3:0] counter;
    
    wire  [97*N_CELL-1:0] out_p;
    wire [N_CELL-1:0] en;
    wire [N_CELL:0] w_en;
    assign w_en = {1'b0,en};
    wire [191:0] exitQueueFIFO;
    assign d_out = exitQueueFIFO;
    wire [97*(N_CELL+1)-1:0]actual_out_p = {{1'b1,{96{1'b0}}},out_p};
    init_axis axis(
    .clk(ap_clk),
    .rst(~ap_rst_n),
    
    .i_init_start            ( MD_state_w == 1                      ),
    .i_dump_start            ( MD_state_w == 2 || MD_state_w == 3   ),
    .i_init_ID               ( init_id_w                            ),
    
    .o_m_axis_k2pc_tvalid    ( k2pc_tvalid                          ),     // To pos caches for init
    .o_m_axis_k2pc_tdata     ( k2pc_tdata                           ),
    
    .o_m_axis_k2h_tvalid     ( k2h_tvalid                           ),     // To host for dump
    .o_m_axis_k2h_tdata      ( k2h_tdata                            ),
    .i_m_axis_k2h_tdata      ( {{285{1'b0}}, exitQueueFIFO}         ),
    .i_s_axis_h2k_tvalid     ( S_AXIS_h2k_tvalid                    ),
    .i_s_axis_h2k_tdata      ( S_AXIS_h2k_tdata                     ),
    .i_s_axis_h2k_tkeep      ( S_AXIS_h2k_tkeep                     ),
    .i_s_axis_h2k_tlast      ( S_AXIS_h2k_tlast                     ),
    .i_s_axis_h2k_tdest      ( S_AXIS_h2k_tdest                     ));
    
    simulator sim(ap_clk,~ap_rst_n,elem_write,d_in[0+:210],out_p,en,initcounter,elem_read,step,elem_write,done,out_pos);
    
    PairExitFIFO ExitFIFO(ap_clk,~ap_rst_n,{{31{1'b0}},w_en[2*counter+:2],actual_out_p[(97*2*counter)+:97*2]},exitQueueFIFO,read_ctrl,out_count);
    
    
    always @(posedge ap_clk) begin
        if(!ap_rst_n) begin
            counter <= 4'd13;
        end else begin
            if(counter == 13) begin
                counter <= 0;
            end else begin
                counter <= counter + 1;
            end
            
            
        end
    end
endmodule
