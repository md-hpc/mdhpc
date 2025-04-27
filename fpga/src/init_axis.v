`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/20/2025 02:34:22 PM
// Design Name: 
// Module Name: init_axis
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


module init_axis  #(
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
       parameter integer TDEST_WIDTH     = 16
)(

    input                                             clk, 
    input                                             rst, 
    input                                             i_init_start,
    input                                             i_dump_start,
    input        [15:0]                               i_init_ID,
    
    input                                             i_s_axis_h2k_tvalid,
    input        [AXIS_TDATA_WIDTH-1:0]               i_s_axis_h2k_tdata,
    input        [AXIS_TDATA_WIDTH/8-1:0]             i_s_axis_h2k_tkeep,
    input                                             i_s_axis_h2k_tlast,
    input        [TDEST_WIDTH-1:0]                    i_s_axis_h2k_tdest,
    
    input       [AXIS_TDATA_WIDTH-1:0]               i_m_axis_k2h_tdata,
    
    output   reg                                    o_m_axis_k2pc_tvalid,
    output  reg [AXIS_TDATA_WIDTH-1:0]               o_m_axis_k2pc_tdata,
    
    output   reg                                    o_m_axis_k2h_tvalid,
    output  reg [AXIS_TDATA_WIDTH-1:0]               o_m_axis_k2h_tdata
);

always@(posedge clk) begin
    if (rst) begin
        o_m_axis_k2h_tvalid     <= 0;
        o_m_axis_k2h_tdata      <= 0;
        o_m_axis_k2pc_tvalid    <= 0;
        o_m_axis_k2pc_tdata     <= 0;
    end
    else begin
        if (i_init_start) begin
            o_m_axis_k2h_tvalid    <= 0;
            o_m_axis_k2h_tdata     <= 0;
            o_m_axis_k2pc_tvalid    <= i_s_axis_h2k_tvalid;
            o_m_axis_k2pc_tdata     <= i_s_axis_h2k_tdata;
        end
        else begin   
             if (i_dump_start) begin
                o_m_axis_k2h_tvalid    <= ~i_m_axis_k2h_tdata[226];
                o_m_axis_k2h_tdata     <= i_m_axis_k2h_tdata;
            end
            else begin
                o_m_axis_k2h_tvalid    <= 0;
                o_m_axis_k2h_tdata     <= 0;
            end
            o_m_axis_k2pc_tvalid    <= 0;
            o_m_axis_k2pc_tdata     <= 0;
        end
    end
end
endmodule
