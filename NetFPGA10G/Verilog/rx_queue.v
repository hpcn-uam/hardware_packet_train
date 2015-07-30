/*******************************************************************************
 *
 *  NetFPGA-10G http://www.netfpga.org
 *
 *  File:
 *        rx_queue.v
 *
 *  Library:
 *        hw/osnt/pcores/osnt_10g_interface_v1_11_a
 *
 *  Module:
 *        rx_queue
 *
 *  Author:
 *        James Hongyi Zeng, Gianni Antichi
 *
 *  Description:
 *        AXI-MAC converter: RX side
 *
 *  Copyright notice:
 *        Copyright (C) 2010, 2011 The Board of Trustees of The Leland Stanford
 *                                 Junior University
 *
 *  Licence:
 *        This file is part of the NetFPGA 10G development base package.
 *
 *        This file is free code: you can redistribute it and/or modify it under
 *        the terms of the GNU Lesser General Public License version 2.1 as
 *        published by the Free Software Foundation.
 *
 *        This package is distributed in the hope that it will be useful, but
 *        WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *        Lesser General Public License for more details.
 *
 *        You should have received a copy of the GNU Lesser General Public
 *        License along with the NetFPGA source package.  If not, see
 *        http://www.gnu.org/licenses/.
 *
 */

module rx_queue
#(
   parameter AXI_DATA_WIDTH = 64, //Only 64 is supported right now.
   parameter TIMESTAMP_WIDTH = 64,
   parameter AXI_CALCULATIONS_DATA_WIDTH = 160,
   parameter C_DEFAULT_SRC_PORT = 0
)
(
   // AXI side
   output reg [AXI_DATA_WIDTH-1:0]  tdata,
   output reg [AXI_DATA_WIDTH/8-1:0]  tstrb,
   output reg tvalid,
   output reg tlast,
   input  tready,

   output pkt_start,

   input clk,
   input reset,

   // MAC side
   input [63:0] rx_data,
   input [ 7:0] rx_data_valid,
   input        rx_good_frame,
   input        rx_bad_frame,
   input clk156,

   // AXI stream for hardware calculations

   output wire [AXI_CALCULATIONS_DATA_WIDTH-1:0] tdata_calculations,
   output wire [(AXI_CALCULATIONS_DATA_WIDTH/8)-1:0] tstrb_calculations,
   output wire tvalid_calculations,
   output wire tlast_calculations,
   input       tready_calculations,

	
   // Timestamp

   input [TIMESTAMP_WIDTH-1:0]	    stamp_counter		
);

   localparam IDLE = 0;
   localparam WAIT_FOR_EOP = 1;
   localparam DROP = 2;

   localparam ERR_IDLE = 0;
   localparam ERR_WAIT = 1;
   localparam ERR_BUBBLE = 2;

   wire fifo_almost_full;
   wire fifo_empty;
   reg  fifo_wr_en;

   wire info_fifo_empty;
   reg  info_fifo_rd_en;
   reg  info_fifo_wr_en;
   wire rx_bad_frame_fifo;

   reg  rx_fifo_rd_en;
   wire [AXI_DATA_WIDTH-1:0]  tdata_delay;
   wire [AXI_DATA_WIDTH/8-1:0]  tstrb_delay;

   reg  [2:0] state, state_next;
   reg  [2:0] err_state, err_state_next;
   reg  err_tvalid;

   reg rx_pkt;
   
   reg [4:0] word;
   reg  [TIMESTAMP_WIDTH-1:0] timestamp;

   // register for hardware calculations


   reg [TIMESTAMP_WIDTH-1:0] timestamp_rx_1,timestamp_rx,timestamp_tx;
   reg [15:0] pkt_length,identificator;
   reg w6,w7;
   
   reg [AXI_CALCULATIONS_DATA_WIDTH-1:0] tdata_calc;
   reg valid;
   
   reg [1:0] state_stream;
   wire ack;
   
   localparam WAIT_FOR_DATA=0; 
   localparam SEND_DATA=1;
   localparam WAIT_ACK =2;

   /* output reg [AXI_CALCULATIONS_DATA_WIDTH-1:0] tdata_calculations_con,
   output reg [(AXI_CALCULATIONS_DATA_WIDTH/8)-1:0] tstrb_calculations_con,
   output reg tvalid_calculations_con,
   output reg tlast_calculations_con,
   input      tready_calculations,*/
   
   
   //assign 

    generate_stream #
    (.C_DEFAULT_SRC_PORT(C_DEFAULT_SRC_PORT))
    calculations(
	.clk(clk),
	.reset(reset),
		
	.data(tdata_calc),
	.valid(valid),
	.ack(ack),
		
	// AXI stream for hardware calculations

	.tdata_calculations(tdata_calculations),
	.tstrb_calculations(tstrb_calculations),
	.tvalid_calculations(tvalid_calculations),
	.tlast_calculations(tlast_calculations),
	.tready_calculations(tready_calculations)

);


   // Instantiate clock domain crossing FIFO
   FIFO36_72 #(
   	.SIM_MODE("FAST"),
   	.ALMOST_FULL_OFFSET(9'd300), // > Ethernet MAX length / 6 = 1516Byte/6 = 252
   	.ALMOST_EMPTY_OFFSET(9'hA),
   	.DO_REG(1),
   	.EN_ECC_READ("FALSE"),
   	.EN_ECC_WRITE("FALSE"),
   	.EN_SYN("FALSE"),
   	.FIRST_WORD_FALL_THROUGH("TRUE")
   	) rx_fifo (
		.ALMOSTEMPTY(),
		.ALMOSTFULL(fifo_almost_full),
		.DBITERR(),
		.DO(tdata_delay),
		.DOP(tstrb_delay),
		.ECCPARITY(),
		.EMPTY(fifo_empty),
		.FULL(),
		.RDCOUNT(),
		.RDERR(),
		.SBITERR(),
		.WRCOUNT(),
		.WRERR(),
		.DI(rx_data),
		.DIP(rx_data_valid),
		.RDCLK(clk),
		.RDEN(rx_fifo_rd_en),
		.RST(reset),
		.WRCLK(clk156),
		.WREN(fifo_wr_en)
   	);

   	small_async_fifo
   	#(
   	  .DSIZE (1),
      .ASIZE (9)
	) rx_info_fifo
        (
         .wdata(rx_bad_frame),
         .winc(info_fifo_wr_en),
         .wclk(clk156),

         .rdata(rx_bad_frame_fifo),
         .rinc(info_fifo_rd_en),
         .rclk(clk),

         .rempty(info_fifo_empty),
         .r_almost_empty(),
         .wfull(),
         .w_almost_full(),
	     .rrst_n(~reset),
         .wrst_n(~reset)
         );


    


	always @(posedge clk) begin
	
	
	if (reset == 1'b1) begin
		w6 <=1'b0;
		w7 <=1'b0;
		timestamp_rx <=64'b0;
		valid <= 1'b0;
		state_stream <= WAIT_FOR_DATA;
	end
		
   	if(rx_fifo_rd_en) begin
		if (word == 2) begin
			pkt_length[7:0]  <=tdata_delay[15:8];
			pkt_length[15:8] <=tdata_delay[7:0];
			identificator[7:0] <=tdata_delay[31:24];
			identificator[15:8] <=tdata_delay[23:16];
			
			tdata <= tdata_delay;
		end
		
		else if (word==5) begin
		
			timestamp_tx[23:16]<= tdata_delay[63:56];	//TS tx 6
			timestamp_tx[31:24]<= tdata_delay[55:48];	//TS tx 5
			timestamp_tx[39:32]<= tdata_delay[47:40];	//TS tx 4
			timestamp_tx[47:40]<= tdata_delay[39:32];	//TS tx 3
			timestamp_tx[55:48]<= tdata_delay[31:24];	//TS tx 2
			timestamp_tx[63:53]<= tdata_delay[23:16];	//TS tx 1*/
			
			
			tdata <= tdata_delay;
		end
   	
   		else if (word==6 && tdata_delay[63:16]==48'hFFFFFFFFFFFF) begin
			tdata[63:56]<=timestamp[23:16];	//TS rx 6
			tdata[55:48]<=timestamp[31:24];	//TS rx 5
			tdata[47:40]<=timestamp[39:32];	//TS rx 4
			tdata[39:32]<=timestamp[47:40];	//TS rx 3
			tdata[31:24]<=timestamp[55:48];	//TS rx 2
			tdata[23:16]<=timestamp[63:53];	//TS rx 1*/
			
			timestamp_tx[7:0]<= tdata_delay[15:8];	//TS tx 8
			timestamp_tx[15:8]<= tdata_delay[7:0];	//TS tx 7	
			
			tdata[15:0] <= tdata_delay[15:0];
			w6 <=1'b1;
			

   		end
   		
   		else if (word==7 && tdata_delay[15:0]==16'hFFFF) begin 
			tdata[63:16] <= tdata_delay[63:16];

			tdata[15:8]<=timestamp[7:0];	//TS rx 8
			tdata[7:0]<=timestamp[15:8];	//TS rx 7	
			
            
            
			w7 <=1'b1;
			timestamp_rx <= timestamp;
			/*if (w6==1'b1) begin
			    timestamp_rx <= timestamp;
				timestamp_rx_1 <= timestamp_rx;
	
			end  */
			
			timestamp_rx_1 <= timestamp_rx;

   		end
   		
   		else begin
   			tdata <= tdata_delay;
   		end 
   		
   		word = word +1;
		tstrb <= tstrb_delay;
	end


	if (tstrb_delay==8'h00) begin
		word = 0;
		timestamp <=stamp_counter;
	end

	
	
	
	
	//tdata_calc <= {{16{1'b0}},pkt_length,timestamp_rx_1,timestamp_rx};
	
	/*if (w6==1'b1 && w7==1'b1) begin // generate stream with data for hardware calculations

		//tdata_calc <= {{16{1'b0}},pkt_length,timestamp_rx_1,timestamp_rx};

		//tdata_calc <= {pkt_length,identificator,timestamp_tx,timestamp_rx};
		
		w6 <=1'b0;
		w7 <=1'b0;
		valid <= 1'b1;
		
	end
	else begin
		valid <= 1'b0;
	end
	*/
	
	
	case (state_stream) 
	
		WAIT_FOR_DATA: begin
		
			if (w6==1'b1 && w7==1'b1 && ack==1'b0) begin
				state_stream <= SEND_DATA;
				valid <= 1'b1;
				tdata_calc <= {pkt_length,identificator,timestamp_tx,timestamp_rx};
				//tdata_calc <= {{16{1'b0}},pkt_length,timestamp_rx_1,timestamp_rx};
				w6 <=1'b0;
				w7 <=1'b0;
			end
		
		end
		
		
		SEND_DATA: begin
			if (ack==1'b1) begin
				state_stream <= WAIT_ACK;
				valid <= 1'b0;
			end	
		
		end
		
		WAIT_ACK: begin
			if (ack==1'b0) begin
				state_stream <= WAIT_FOR_DATA;
			end
		
		end
		
		default: begin
			state_stream <= WAIT_FOR_DATA;
		
		end
	
	endcase
	



  end
	
     
     
/* end insert for mario*/     

// -------

// clock domain crossing for the pkt_start puls


        sync_pulse
        	sync_pulse
        (
         .clkA(clk156),
         .rstA(reset),
         .pulseA(rx_pkt),

         .clkB(clk),
         .rstB(reset),
         .pulseB(pkt_start),

         .pulseA_busy()
         );

// --------

     always @* begin
         state_next = state;
         fifo_wr_en = 1'b0;
         info_fifo_wr_en = 1'b0;
	 rx_pkt = 1'b0;

         case(state)
             IDLE: begin
                 if(rx_data_valid == 8'hFF) begin
                     info_fifo_wr_en = 1'b1;
                     if(~fifo_almost_full) begin
                         fifo_wr_en = 1'b1;
			 rx_pkt	 = 1'b1;
                         state_next = WAIT_FOR_EOP;
                     end
                     else begin
                         state_next = DROP;
                     end
                 end
             end

             WAIT_FOR_EOP: begin
                 fifo_wr_en = 1'b1;
                 if(rx_data_valid == 8'h0) begin  // Make sure there is a bubble between packets
                     state_next = IDLE;
                 end
             end

             DROP: begin
                 if(rx_data_valid != 8'hFF) begin
                     state_next = IDLE;
                 end
             end
         endcase
     end

     always @* begin
         info_fifo_rd_en = 0;
         err_state_next = err_state;
         err_tvalid = 0;

         rx_fifo_rd_en = 0;
         tlast = 0;
         tvalid = 0;

         case(err_state)
             ERR_IDLE: begin
                 rx_fifo_rd_en = (~fifo_empty & tready);
                 tvalid = (~fifo_empty);
                 if(tstrb_delay == 8'h0 & ~fifo_empty) begin // End of the packet
                     rx_fifo_rd_en = 0;
                     tvalid = 0;
                     err_state_next = ERR_WAIT;
                 end
             end
             ERR_WAIT: begin
                 if(~info_fifo_empty) begin
                 	tlast = 1;
                 	tvalid = 1;
                 	if(tready) begin
                     	info_fifo_rd_en = 1;
                     	rx_fifo_rd_en = 1;
                     	err_tvalid = rx_bad_frame_fifo;
                     	err_state_next = ERR_BUBBLE;
                    end
                 end
             end
             ERR_BUBBLE: begin
                 if(~fifo_empty) begin // Head of the packet
                     rx_fifo_rd_en = 1;
                     err_state_next = ERR_IDLE;
                 end
             end
         endcase
     end

     always @(posedge clk156 or posedge reset) begin
         if(reset) begin
             state <= IDLE;
         end
         else begin
             state <= state_next;
         end
     end
     always @(posedge clk or posedge reset) begin
         if(reset) begin
             err_state <= ERR_BUBBLE;
         end
         else begin
             err_state <= err_state_next;
         end
     end
endmodule