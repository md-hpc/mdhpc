`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/02/2025 01:36:47 PM
// Design Name: 
// Module Name: top_tb
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


module top_tb(

    );
    integer i;
    reg [255:0] particles [299:0];
integer j;
    reg clk, fast_clk, reset, data_in_ready;
    reg read_ctrl;
    wire elem_read;
    reg [255:0] data_in;
    wire [191:0] d_out;
    reg [31:0] step;
    
    reg [10:0] num_in;
    //simulator sim(.fast_clk(fast_clk),.reset(reset),.data_in_ready(data_in_ready),.data_in(data_in),.step(0),.elem_write(data_in_ready));
    MD_Wrapper md(.ap_clk(fast_clk),.ap_rst_n(~reset),.d_in(data_in[0+:210]),.d_out(d_out),.elem_write(data_in_ready),.step(step),.read_ctrl(read_ctrl),.elem_read(elem_read));
    initial begin
    fast_clk = 0;
    clk = 0;
    reset = 1;
    data_in_ready = 0;
    read_ctrl = 0;
    num_in = 0;
    step = 0;
    data_in = 0;
    $readmemh("C:/Users/fadik/Documents/BU/EC464/RTL_MD/RTL_MD.srcs/sim_1/new/BRAM_INIT.txt", particles);
    #160 reset = 0;
    #260
    
    #400
    for(i = 0; i < 300; i=i+1)begin
    #100
    data_in_ready = 0;
        #32 data_in = particles[i];
        #64
        data_in_ready = 1;
    end
    #100
    #32
    data_in_ready = 0;
    //$finish;
    
    #20000
    
    for(i = 0; i < 300; i=i+1)begin
        #513 read_ctrl = 1;
        #534 read_ctrl = 0;
    end
    step = 1;
    end
    
    always #1 fast_clk = ~fast_clk;
    always #16 clk = ~clk;
    
    always begin
    #32
    for(j = 0; j < 28; j=j+1)begin
    num_in = num_in + md.w_en[j];
    end
    
    end
    
endmodule
