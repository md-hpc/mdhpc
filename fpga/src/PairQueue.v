`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/13/2025 06:26:05 PM
// Design Name: 
// Module Name: PairQueue
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

//NSIZE = 14 / number of neighbor cells
module PairQueue(
    input  clk,
    input  reset,
    input  [226:0] in,
    output [226:0] out,
    output qempty
    );
    genvar i;
    reg emp;
    reg [3:0] counter;
    
    wire [226:0] im_out;
    //wire [193:0] inputs [13:0];
    reg delayed_emp;
    
    wire q_empty, wr_en, rd_en, q_full;
    
    wire [226:0] data_in;
    /*
    generate
        for(i = 0; i < 14; i = i + 1) begin
            assign inputs[i] = in[194*i+:194];
        end  
    endgenerate
    */
    assign qempty = emp;
    assign data_in = in;
    assign wr_en = (counter < 14 && in[226] == 0);
    assign rd_en = counter == 15;
    assign out = (emp)? {1'b1,{226{1'b0}}}:im_out;
    PQFIFO pq(.empty(q_empty),.srst(reset),.clk(clk),.din(in),.wr_en(wr_en),.full(q_full),.rd_en(rd_en),.dout(im_out));
    
    always @(posedge clk) begin
        if(reset) begin
            counter <= 4'd15;
            emp <= q_empty;
            delayed_emp <= 1;
        end else begin
            if(counter == 15) begin
                counter <= 0;
                emp <= q_empty;
                if(delayed_emp == 1) begin
                    delayed_emp <= q_empty;
                end else begin
                    delayed_emp <= emp;
                end
            end else begin
                counter <= counter + 1;
            end
            
        end
    end
    
    
    /*
    // New Logic
    reg [3:0] queue_write_ptr;
    reg [3:0] queue_read_ptr;
    
    wire [3:0] queue_read_next_ptr;
    wire [3:0] queue_write_next_ptr;
    wire [3:0] queue_write_next_alt_ptr;
    
    wire [13:0] wr_en;
    wire [13:0] rd_en;
    
    genvar i;
    integer iter;
    reg [193:0] queue [31:0];
    wire [7:0] indexor [13:0];
    
    wire [193:0] in_w [13:0];
    wire [193:0] data_in [13:0];
    wire [193:0] data_out [13:0];
    wire [13:0] cascade;
    
    
    wire [13:0] q_empty;
    wire [13:0] q_full;
    
    
    
    assign rd_en = {1'b1,{13{1'b0}}} >> queue_read_ptr;
    assign in_w[0] = in[0+:193];
    assign indexor[0] = (in_w[0][193]==0);
    assign cascade[0] = ~in_w[0][193];
    
    assign out[193] = (q_empty[queue_read_ptr] == 1'b1 || data_out[queue_read_ptr][193] == 1'b1)? 1'b1:1'b0;
    assign out[192:0] = data_out[queue_read_ptr][192:0];
        
    generate
        for(i = 0; i < 14; i=i+1)begin
            pQueue pair_queue(.empty(q_empty[i]),.srst(reset),.clk(clk),.din(data_in[i]),.wr_en(wr_en[i]),.full(q_full),.rd_en(rd_en[i]),.dout(data_out[i]));
            assign wr_en[i] = (queue_write_ptr == 0)? (in_w[i][193]==0):
                              (queue_write_ptr == 1)? (in_w[(i+12)%14][193]==0):
                              (queue_write_ptr == 2)? (in_w[(i+11)%14][193]==0):
                              (queue_write_ptr == 3)? (in_w[(i+10)%14][193]==0):
                              (queue_write_ptr == 4)? (in_w[(i+9)%14][193]==0):
                              (queue_write_ptr == 5)? (in_w[(i+8)%14][193]==0):
                              (queue_write_ptr == 6)? (in_w[(i+7)%14][193]==0):
                              (queue_write_ptr == 7)? (in_w[(i+6)%14][193]==0):
                              (queue_write_ptr == 8)? (in_w[(i+5)%14][193]==0):
                              (queue_write_ptr == 9)? (in_w[(i+4)%14][193]==0):
                              (queue_write_ptr == 10)? (in_w[(i+3)%14][193]==0):
                              (queue_write_ptr == 11)? (in_w[(i+2)%14][193]==0):(in_w[(i+1)%14][193]==0);
                              
            assign data_in[i] = (queue_write_ptr == 0)? (in_w[i]):
                              (queue_write_ptr == 1)? (in_w[(i+12)%14]):
                              (queue_write_ptr == 2)? (in_w[(i+11)%14]):
                              (queue_write_ptr == 3)? (in_w[(i+10)%14]):
                              (queue_write_ptr == 4)? (in_w[(i+9)%14]):
                              (queue_write_ptr == 5)? (in_w[(i+8)%14]):
                              (queue_write_ptr == 6)? (in_w[(i+7)%14]):
                              (queue_write_ptr == 7)? (in_w[(i+6)%14]):
                              (queue_write_ptr == 8)? (in_w[(i+5)%14]):
                              (queue_write_ptr == 9)? (in_w[(i+4)%14]):
                              (queue_write_ptr == 10)? (in_w[(i+3)%14]):
                              (queue_write_ptr == 11)? (in_w[(i+2)%14]):(in_w[(i+1)%14]);
        end
        for(i = 1; i < 14; i=i+1)begin
            assign in_w[i] = in[(i*193)+:193];
            assign indexor[i] = indexor[i-1] + (in_w[i][193]==0);
            assign cascade[i] = cascade[i-1] | (~in_w[i][193]);
        end
    endgenerate
    assign queue_read_next_ptr = (queue_read_ptr+1)%14;
    assign queue_write_next_ptr = (queue_write_ptr+indexor[13])%14;
    assign queue_write_next_alt_ptr = (queue_write_ptr+indexor[13] > 0)?(queue_write_ptr+indexor[13] - 1)%14:queue_write_next_ptr;
    assign qempty = q_empty[queue_read_next_ptr] == 0 & indexor[13] == 0;
    
    always @(posedge clk, posedge reset) begin
        if(reset) begin
            queue_write_ptr <= 0;
            queue_read_ptr <= 0;
        end else begin
            //If read_ptr+1 queue is empty and #valid inputs>0, 
            if(q_empty[queue_read_next_ptr] == 1) begin
                queue_read_ptr <= queue_read_next_ptr;
                if(indexor[13]>0) begin
                    queue_write_ptr <= queue_write_next_ptr;
                end
            end else if (indexor[13] > 0) begin
                queue_write_ptr <= queue_write_next_alt_ptr;
            end
        end
    end
    */
    // End of New Logic
    
    /*
    //genvar i;
    genvar i;
    integer iter;
    reg [193:0] queue [31:0];
    
    reg [7:0] queue_start;
    wire [7:0] indexor [13:0];
    
    wire [193:0] in_w [13:0];
    
    wire [13:0] cascade;
    
    
    
    
    
    
    
    assign in_w[0] = in[0+:193];
    assign indexor[0] = queue_start + (in_w[0][193]==0);
    assign cascade[0] = ~in_w[0][193];
    generate 
        for(i = 1; i < 14; i=i+1)begin
            assign in_w[i] = in[(i*193)+:193];
            assign indexor[i] = indexor[i-1] + (in_w[i][193]==0);
            assign cascade[i] = cascade[i-1] | (~in_w[i][193]);
        end
    endgenerate
    
    always @ (posedge clk, posedge reset) begin
        if(reset) begin
            for( iter = 0; iter < 256; iter=iter+1) begin
                queue[iter][193] <= 1;
            end
            queue_start <= 0;
        end else begin
            for(iter = 0; iter < 14; iter = iter + 1) begin
                if(in_w[iter][193]) begin
                    if(iter == 0)begin
                        out <= in_w[iter];
                    end else begin
                        if(cascade[iter-1] == 0) begin
                            out <= in_w[iter];
                        end else begin
                            queue[queue_start + indexor[iter] - 2] <= in_w[iter];
                        end
                    end
                end
            
            end
            //
            if(indexor[13] - queue_start == 0) begin
                if(queue_start > 0) begin
                    out <= queue[queue_start - 1];
                    queue_start <= queue_start - 1;
                end else begin
                    out[193] <= 1'b1;
                end
            end else begin
                queue_start <= queue_start + indexor[13] - 1;
            end
        end
        
    end
    */
    
endmodule
