`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/19/2025 02:57:38 PM
// Design Name: 
// Module Name: MD_RL
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


module MD_RL #(
      // Width of S_AXIS_n2k and M_AXIS_k2n interfaces
      parameter integer AXIS_TDATA_WIDTH      = 512,
      // Width of M_AXIS_summary interface
      parameter integer AXIS_SUMMARY_WIDTH    = 128,
      // Width of TDEST address bus
      parameter integer STREAMING_TDEST_WIDTH =  16,
      // Width of S_AXIL data bus
      parameter integer AXIL_DATA_WIDTH       = 32,
      // Width of S_AXIL address bus
      parameter integer AXIL_ADDR_WIDTH       =  9
)(
    // System clocks and resets
    input  wire                                 ap_clk,
    input  wire                                 ap_rst_n,
    /*
    // AXI4-Stream network layer to streaming kernel 
    input  wire       [AXIS_TDATA_WIDTH-1:0]    S_AXIS_n2k_pos_tdata,
    input  wire     [AXIS_TDATA_WIDTH/8-1:0]    S_AXIS_n2k_pos_tkeep,
    input  wire                                 S_AXIS_n2k_pos_tvalid,
    input  wire                                 S_AXIS_n2k_pos_tlast,
    input  wire  [STREAMING_TDEST_WIDTH-1:0]    S_AXIS_n2k_pos_tdest,
    output wire                                 S_AXIS_n2k_pos_tready,
    // AXI4-Stream streaming kernel to network layer
    output wire       [AXIS_TDATA_WIDTH-1:0]    M_AXIS_k2n_pos_tdata,
    output wire     [AXIS_TDATA_WIDTH/8-1:0]    M_AXIS_k2n_pos_tkeep,
    output wire                                 M_AXIS_k2n_pos_tvalid,
    output wire                                 M_AXIS_k2n_pos_tlast,
    output wire  [STREAMING_TDEST_WIDTH-1:0]    M_AXIS_k2n_pos_tdest,
    input  wire                                 M_AXIS_k2n_pos_tready,

    // AXI4-Stream network layer to streaming kernel 
    input  wire       [AXIS_TDATA_WIDTH-1:0]    S_AXIS_n2k_frc_tdata,
    input  wire     [AXIS_TDATA_WIDTH/8-1:0]    S_AXIS_n2k_frc_tkeep,
    input  wire                                 S_AXIS_n2k_frc_tvalid,
    input  wire                                 S_AXIS_n2k_frc_tlast,
    input  wire  [STREAMING_TDEST_WIDTH-1:0]    S_AXIS_n2k_frc_tdest,
    output wire                                 S_AXIS_n2k_frc_tready,
    // AXI4-Stream streaming kernel to network layer
    output wire       [AXIS_TDATA_WIDTH-1:0]    M_AXIS_k2n_frc_tdata,
    output wire     [AXIS_TDATA_WIDTH/8-1:0]    M_AXIS_k2n_frc_tkeep,
    output wire                                 M_AXIS_k2n_frc_tvalid,
    output wire                                 M_AXIS_k2n_frc_tlast,
    output wire  [STREAMING_TDEST_WIDTH-1:0]    M_AXIS_k2n_frc_tdest,
    input  wire                                 M_AXIS_k2n_frc_tready,
    */
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
    
    input  wire      [AXIL_ADDR_WIDTH-1 : 0]    S_AXIL_AWADDR,
    input  wire                                 S_AXIL_AWVALID,
    output wire                                 S_AXIL_AWREADY,
    input  wire      [AXIL_DATA_WIDTH-1 : 0]    S_AXIL_WDATA,
    input  wire  [(AXIL_DATA_WIDTH/8)-1 : 0]    S_AXIL_WSTRB,
    input  wire                                 S_AXIL_WVALID,
    output wire                                 S_AXIL_WREADY,
    output wire                      [1 : 0]    S_AXIL_BRESP,
    output wire                                 S_AXIL_BVALID,
    input  wire                                 S_AXIL_BREADY,
    input  wire      [AXIL_ADDR_WIDTH-1 : 0]    S_AXIL_ARADDR,
    input  wire                                 S_AXIL_ARVALID,
    output wire                                 S_AXIL_ARREADY,
    output wire      [AXIL_DATA_WIDTH-1 : 0]    S_AXIL_RDATA,
    output wire                      [1 : 0]    S_AXIL_RRESP,
    output wire                                 S_AXIL_RVALID,
    input  wire                                 S_AXIL_RREADY
);


wire [9:0] initcounter;

 wire [209:0] d_in;
 wire [191:0] d_out;
 wire elem_read;
 wire read_ctrl;
 wire [31:0] step;
 wire done;
 
 wire debug_reset_n;
 wire elem_write;
 wire [31:0] out_count;
 
 wire [96:0] out_pos;
MD_Wrapper inst_MD_wrapper (
    .ap_clk(ap_clk),
    .ap_rst_n(ap_rst_n & debug_reset_n),
    
    .S_AXIS_h2k_tdata(S_AXIS_h2k_tdata),
    .S_AXIS_h2k_tkeep(S_AXIS_h2k_tkeep),
    .S_AXIS_h2k_tvalid(S_AXIS_h2k_tvalid),
    .S_AXIS_h2k_tlast(S_AXIS_h2k_tlast),
    .S_AXIS_h2k_tdest(S_AXIS_h2k_tdest),
    .S_AXIS_h2k_tready(S_AXIS_h2k_tready),
    
    .M_AXIS_k2h_tdata(M_AXIS_k2h_tdata),
    .M_AXIS_k2h_tkeep(M_AXIS_k2h_tkeep),
    .M_AXIS_k2h_tvalid(M_AXIS_k2h_tvalid),
    .M_AXIS_k2h_tlast(M_AXIS_k2h_tlast),
    .M_AXIS_k2h_tdest(M_AXIS_k2h_tdest),
    .M_AXIS_k2h_tready(M_AXIS_k2h_tready),
    
    .MD_state_w(MD_state_w),
    .init_id_w(init_id_w),
    .initcounter(initcounter),
    .elem_read(elem_read),
    .read_ctrl(read_ctrl),
    .step(step),
    .done(done),
    .d_in(d_in),
    .d_out(d_out),
    .elem_write(elem_write),
    .out_count(out_count),
    .out_pos(out_pos)
    
);

axi4lite #(
        .AXIL_DATA_WIDTH(AXIL_DATA_WIDTH),
        .AXIL_ADDR_WIDTH  (AXIL_ADDR_WIDTH),
        .STREAMING_TDEST_WIDTH(STREAMING_TDEST_WIDTH)
    ) axi4lite_i (
        .S_AXIL_ACLK                ( ap_clk                  ),
        .S_AXIL_ARESETN             ( ap_rst_n                ),
        .S_AXIL_AWADDR              ( S_AXIL_AWADDR           ),
        .S_AXIL_AWVALID             ( S_AXIL_AWVALID          ),
        .S_AXIL_AWREADY             ( S_AXIL_AWREADY          ),
        .S_AXIL_WDATA               ( S_AXIL_WDATA            ),
        .S_AXIL_WSTRB               ( S_AXIL_WSTRB            ),
        .S_AXIL_WVALID              ( S_AXIL_WVALID           ),
        .S_AXIL_WREADY              ( S_AXIL_WREADY           ),
        .S_AXIL_BRESP               ( S_AXIL_BRESP            ),
        .S_AXIL_BVALID              ( S_AXIL_BVALID           ),
        .S_AXIL_BREADY              ( S_AXIL_BREADY           ),
        .S_AXIL_ARADDR              ( S_AXIL_ARADDR           ),
        .S_AXIL_ARVALID             ( S_AXIL_ARVALID          ),
        .S_AXIL_ARREADY             ( S_AXIL_ARREADY          ),
        .S_AXIL_RDATA               ( S_AXIL_RDATA            ),
        .S_AXIL_RRESP               ( S_AXIL_RRESP            ),
        .S_AXIL_RVALID              ( S_AXIL_RVALID           ),
        .S_AXIL_RREADY              ( S_AXIL_RREADY           ),
        .ap_done                    ( ap_done_w               ),
        .ap_idle                    ( ap_idle_w               ),
        .ap_start                   ( ap_start_w              ),
        .MD_state                   ( MD_state_w              ),
        .iter_target                ( iter_target_w           ),
        .init_id                    ( init_id_w               ),
        .init_step                  ( init_step_w             ),
        .dest_id                    ( dest_id_w               ),
        .number_packets             ( number_packets_w        ),
        .initcounter                ( initcounter             ),
        .elem_read(elem_read),
        .read_ctrl(read_ctrl),
        .step(step),
        .done(done),
        .d_in(d_in),
        .d_out(d_out),
        .elem_write(elem_write),
        .debug_reset_n(debug_reset_n),
        .out_count(out_count),
        .out_pos(out_pos)//,
        //.reset_fsm_n                ( reset_fsm_n_w           )
    );

endmodule
