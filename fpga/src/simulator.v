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
module simulator#(parameter N_CELL = 27, parameter N_PARTICLES = 300)(
//input clk,
input fast_clk,
input reset,

input data_in_ready,
input [209:0]data_in,


output  [97*N_CELL-1:0] out_p,
output [N_CELL-1:0] en,
output [9:0] initcounter,
output elem_read,
input [31:0] step,
input elem_write,
output done,
output [96:0]out_pos
);

reg [209:0] prev_in;
 (* keep = "true" *) reg clk;
reg [15:0] counter;
 (* keep = "true" *) wire double_buffer;

reg phase1_done;
reg phase3_done;
assign done = phase3_done;
reg mem_set;
wire phase1_done_w;
wire [N_CELL:0] phase3_done_w_out;
wire [N_CELL:0] phase3_done_w_acc;
wire [N_CELL:0] phase3_done_w;

wire phase3_ready;
wire phase1_ready;

(* keep = "true" *)wire [N_CELL-1:0] p1_wea;
(* keep = "true" *)wire [32*N_CELL-1:0] p1_addra;
(* keep = "true" *)wire [32*N_CELL-1:0] p1_addrb;
(* keep = "true" *)wire [97*N_CELL-1:0] p1_v_dina;
(* keep = "true" *)wire [97*N_CELL-1:0] p1_v_doutb;
(* keep = "true" *)wire [32*N_CELL-1:0] p1_v_iaddr;


(* keep = "true" *)wire [97*N_CELL-1:0] p1_p_doutb;


(* keep = "true" *)wire [N_CELL-1:0] p3_wea;
(* keep = "true" *)wire [32*N_CELL-1:0] p3_addra;
(* keep = "true" *)wire [32*N_CELL-1:0] p3_addrb;

(* keep = "true" *)wire [97*N_CELL-1:0] p3_v_dina;
(* keep = "true" *)wire [97*N_CELL-1:0] p3_v_doutb;

(* keep = "true" *)wire [97*N_CELL-1:0] p3_p_dina;
(* keep = "true" *)wire [97*N_CELL-1:0] p3_p_doutb;

(* keep = "true" *)wire [N_CELL-1:0] v_wea;
(* keep = "true" *)wire [8:0] v_addra[N_CELL-1:0];
(* keep = "true" *)wire [96:0] v_dina[N_CELL-1:0];
(* keep = "true" *)wire [8:0] v_addrb[N_CELL-1:0];
(* keep = "true" *)wire [96:0] v_doutb[N_CELL-1:0];

(* keep = "true" *)wire [N_CELL-1:0] p_wea;
(* keep = "true" *)wire [8:0] p_addra[N_CELL-1:0];
(* keep = "true" *)wire [96:0] p_dina[N_CELL-1:0];
(* keep = "true" *)wire [8:0] p_addrb[N_CELL-1:0];
(* keep = "true" *)wire [96:0] p_doutb[N_CELL-1:0];

reg [7:0] phase1_delay_counter;
reg [209:0] data_in_reg;
reg data_in_ready_reg, data_in_ready_prev;
wire data_in_ready_pulse;


assign out_p = p3_p_dina;
wire phase1_actualy_done = phase1_delay_counter> 98 && phase1_done ? 1'b1 : 1'b0;
(* keep_hierarchy = "yes" *)
ControlUnit CU(.clk(clk),.reset(reset),.phase1_done(phase1_actualy_done),.phase3_done(phase3_done),.mem_set(mem_set),.double_buffer(double_buffer),.phase3_ready(phase3_ready),.phase1_ready(phase1_ready),.step(step));
(* keep_hierarchy = "yes" *)
phase_1 phase1(.clk(clk),.fast_clk(fast_clk),.reset(reset | (~mem_set)),.CTL_DONE(phase1_done_w),.CTL_DOUBLE_BUFFER(double_buffer),.CTL_READY(phase1_ready),.oaddr(p1_addra),.iaddr(p1_addrb),.r_p_caches(p1_p_doutb),.r_v_caches(p1_v_doutb),.w_v_caches(p1_v_dina),.wr_en(p1_wea),.v_iaddr(p1_v_iaddr));
phase_3 phase3(.clk(clk),.reset(reset | (~mem_set)),.CTL_DONE(phase3_done_w_out),.CTL_DOUBLE_BUFFER(double_buffer),.CTL_READY(phase3_ready),.wr_en(p3_wea),.oaddr(p3_addrb),.iaddr(p3_addra),.r_p_caches(p3_p_doutb),.w_p_caches(p3_p_dina),.r_v_caches(p3_v_doutb),.w_v_caches(p3_v_dina));


reg [9:0] init_counter;

assign initcounter = init_counter;
assign phase3_done_w_acc[0] = phase3_done_w_out[0];
assign phase3_done_w = phase3_done_w_acc[N_CELL];

assign elem_read = prev_in == data_in;
assign out_pos = p_doutb[data_in[192+:8]];
genvar i;
generate
for(i = 0; i < N_CELL; i = i + 1) begin
    assign en[i] = phase3_ready & p3_wea[i] &(p_dina[i][96] != 1);
    assign p_wea[i] = phase1_ready? 1'b0 : 
                      phase3_ready? p3_wea[i]: data_in_ready & data_in[192+:8] == i & elem_write;
    assign p_addra[i] = phase1_ready? p1_addra[i*32+:32] : 
                      phase3_ready? p3_addra[i*32+:32] : data_in[200+:9];
    assign p_addrb[i] = phase1_ready? p1_addrb[i*32+:32] : 
                      (phase3_ready && phase3_done == 0)? p3_addrb[i*32+:32] :
                      (phase3_ready && phase3_done == 1)? data_in[200+:9] : 0;
    assign p_dina[i] = phase1_ready? {1'b0,data_in[0+:96]} : 
                      phase3_ready? p3_p_dina[i*97+:97] : {1'b0,data_in[0+:96]};
    //assign p_doutb[i] = phase1_ready? p1_p_doutb[i*97+:97] : 
    //                 phase3_ready? p3_p_doutb[i*97+:97] : 0;
    assign p1_p_doutb[i*97+:97] = phase1_ready? p_doutb[i] : 0;
    assign p3_p_doutb[i*97+:97] = phase3_ready? p_doutb[i] : 0;
                      
    assign v_wea[i] = phase1_ready? p1_wea[i] : 
                      phase3_ready? p3_wea[i] : data_in_ready & data_in[192+:8] == i & elem_write;
    assign v_addra[i] = phase1_ready? p1_v_iaddr[i*32+:32] : 
                      phase3_ready? p3_addra[i*32+:32] : data_in[200+:9];
    assign v_addrb[i] = phase1_ready? p1_addra[i*32+:32] : 
                      phase3_ready? p3_addrb[i*32+:32] : 0;
    assign v_dina[i] = phase1_ready? p1_v_dina[i*97+:97] : 
                      phase3_ready? p3_v_dina[i*97+:97] : {1'b0,data_in[96+:96]};
    //assign v_doutb[i] = phase1_ready? p1_p_doutb[i*97+:97] : 
    //                  phase3_ready? p3_p_doutb[i*97+:97] : 0;
    assign p1_v_doutb[i*97+:97] = phase1_ready? v_doutb[i] : 0;
    assign p3_v_doutb[i*97+:97] = phase3_ready? v_doutb[i] : 0;
    BlockRAM PBRAM(.clka(clk),.clkb(clk),.ena(1'b1),.wea(p_wea[i]),.addra(p_addra[i]),.dina(p_dina[i]),.enb(1'b1),.addrb(p_addrb[i]),.doutb(p_doutb[i]));
    BlockRAM VBRAM(.clka(clk),.clkb(clk),.ena(1'b1),.wea(v_wea[i]),.addra(v_addra[i]),.dina(v_dina[i]),.enb(1'b1),.addrb(v_addrb[i]),.doutb(v_doutb[i]));
    
end
for(i = 1; i < N_CELL+1; i = i + 1) begin
    assign phase3_done_w_acc[i] = phase3_done_w_acc[i-1] & phase3_done_w_out[i];
end
endgenerate

always @(posedge fast_clk, posedge reset) begin
if(reset) begin
    counter <= 0;
    clk = 0;
end else begin
    if(counter == 7) begin
        counter <= 0;
        clk = ~clk;
    end else begin
        counter <= counter + 1;
    end

end

end
always @(posedge clk) begin
    data_in_reg <= data_in;
    data_in_ready_reg <= data_in_ready;
    data_in_ready_prev <= data_in_ready_reg;
end

assign data_in_ready_pulse = data_in_ready_reg && ~data_in_ready_prev;

always @(posedge clk, posedge reset) begin
    if(reset) begin
        phase1_done <= 0;
        phase3_done <= 0;
        init_counter <= 0;
        phase1_delay_counter <= 0;
        mem_set <= 0;
        prev_in <= 0;
    end else begin
        if(phase3_done == 1) begin
            phase1_delay_counter <= 0;
        end else if(phase1_delay_counter < 100) begin
            phase1_delay_counter <= phase1_delay_counter + 1;
        end
    
    
        if(init_counter < N_PARTICLES) begin
            if(data_in_ready && prev_in != data_in_reg && elem_write && data_in_ready_pulse ) begin
                init_counter <= init_counter + 1;
                prev_in <= data_in_reg;
            end
        end else begin
            mem_set <= 1;
            phase1_done <= phase1_done_w;
            phase3_done <= phase3_done_w[0];
        end
    end




end




endmodule