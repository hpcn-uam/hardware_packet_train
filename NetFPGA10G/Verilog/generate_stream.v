/*******************************************************************************
 *
 *  NetFPGA-10G http://www.netfpga.org
 *
 *  File:
 *        generate_stream.v
 *
 *  Library:
 *        hw/osnt/pcores/osnt_10g_interface_v1_11_a
 *
 *  Module:
 *        rx_queue
 *
 *  Author:
 *        Mario Ruiz
 *
 *
 *  Copyright (C) 2015 - HPCN-UAM High Performance Computing and Networking
 *
 *  Licence:
 *        This file is part of the HPCN-NetFPGA 10G development base package.
 *
 *        This file is free code: you can redistribute it and/or modify it under
 *        the terms of the GNU Lesser General Public License version 2.0 as
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

#(
   parameter AXI_CALCULATIONS_DATA_WIDTH = 160,
   parameter C_DEFAULT_SRC_PORT = 0

)
(
	input clk,
	input reset,
		
	input [AXI_CALCULATIONS_DATA_WIDTH-1:0] data,
	input valid,
	output reg ack,
		
	// AXI stream for hardware calculations

	output reg [AXI_CALCULATIONS_DATA_WIDTH-1:0] tdata_calculations,
	output reg [(AXI_CALCULATIONS_DATA_WIDTH/8)-1:0] tstrb_calculations,
	output reg tvalid_calculations,
	output reg tlast_calculations,
	input       tready_calculations

);
	
	localparam WAIT=0;
	localparam SND=1;
	
	reg wr_en,rd_en;
	
	wire fifo_empty,fifo_info_empty,fifo_rx_empty,fifo_tx_empty;
	
	wire [63:0] rx_1,rx,pkt_info;

	reg state; 
	
	
	reg [1:0] state_wr_fifo;
	
	localparam WAIT_DATA=0;
	localparam SND_ACK=1;
	localparam WRITE=2;
	
	
wire [35:0] CONTROL0;

	generate 
		begin
		if(C_DEFAULT_SRC_PORT == 8'h10)
			begin
				

				chipscope_icon ICON (
					.CONTROL0(CONTROL0) // INOUT BUS [35:0]
				);

				chipscope_ila ILA(
					.CONTROL(CONTROL0), // INOUT BUS [35:0]
					.CLK(clk), // IN
					.TRIG0(tdata_calculations),  // IN BUS [160:0]
					.TRIG1(tstrb_calculations),  // IN BUS [20:0]
					.TRIG2(tvalid_calculations), // IN BUS 
					.TRIG3(tlast_calculations),  // IN BUS 
					.TRIG4(tready_calculations), // OUT BUS 
					.TRIG5(data),		     // IN BUS [159:0]	
					.TRIG6(valid),		     // IN BUS
					.TRIG7(ack)		     // OUT BUS
				);
		
			end
		end
		
	endgenerate		


	/* save data  in fifo */
	
	always @(posedge clk) begin
	
		if (reset == 1'b1) begin
			wr_en <= 1'b0;
			state_wr_fifo <= WAIT_DATA;
			ack <=1'b0;
	
		end
	
	
		case (state_wr_fifo)
		
			WAIT_DATA: begin
				wr_en <= 1'b0;
				
				if(valid==1'b1) begin
					state_wr_fifo <= SND_ACK;
					ack <=1'b1;
				end
				
			
			end
			
			SND_ACK: begin
				if(valid==1'b0) begin
					state_wr_fifo <= WRITE;
					ack <=1'b0;
				end
			
			end
			
			WRITE: begin
				wr_en <= 1'b1;
				state_wr_fifo <= WAIT_DATA;
			
			end
			
			
			default: begin
				wr_en <= 1'b0;
				state_wr_fifo <= WAIT_DATA;
			
			end
		
		
		endcase	
	
	end
    
    
	/* write data in stream */
    
	assign fifo_empty = fifo_info_empty || fifo_rx_empty || fifo_tx_empty;
    
	always @(posedge clk) begin
	
		if (reset==1'b1) begin 
		state <= WAIT;
		rd_en <= 1'b0;
		tvalid_calculations  <= 1'b0;
		tlast_calculations   <= 1'b0;   
		end
		
		case (state)
		
		WAIT: begin
			
			tdata_calculations  <= {160{1'b0}};
			tstrb_calculations <= {20{1'b0}};
			tvalid_calculations <= 1'b0;
			tlast_calculations  <= 1'b0;
		
			if (fifo_empty == 1'b0 && tready_calculations==1'b1) begin
				rd_en <= 1'b1;
				state <= SND;
			end
			
		end    
		
		SND: begin
			
			//if (tready_calculations==1'b1) begin
				tdata_calculations  <= {pkt_info[31:0],rx_1,rx};
				tstrb_calculations <= {20{1'b1}};
				tvalid_calculations <= 1'b1;
				tlast_calculations  <= 1'b1; 
				state <= WAIT;
			//end    
			rd_en <= 1'b0;
		
		end
		default:
		begin
			
			tdata_calculations  <= {160{1'b0}};
			tstrb_calculations <= {20{1'b0}};
			tvalid_calculations <= 1'b0;
			tlast_calculations  <= 1'b0;
			rd_en <= 1'b0;
			state <= WAIT;
		
		end   
		
		
		endcase    
		
		
	
	
	end 
   	

   	
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
   	) info_fifo (
		.ALMOSTEMPTY(),
		.ALMOSTFULL(),
		.DBITERR(),
		.DO(pkt_info),
		.DOP(),
		.ECCPARITY(),
		.EMPTY(fifo_info_empty),
		.FULL(),
		.RDCOUNT(),
		.RDERR(),
		.SBITERR(),
		.WRCOUNT(),
		.WRERR(),
		.DI({{32 {1'b0}} , data[159:128]}),
		.DIP(8'h0000FFFF),
		.RDCLK(clk),
		.RDEN(rd_en),
		.RST(reset),
		.WRCLK(clk),
		.WREN(wr_en)
   	);
   	
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
		.ALMOSTFULL(),
		.DBITERR(),
		.DO(rx_1),
		.DOP(),
		.ECCPARITY(),
		.EMPTY(fifo_tx_empty),
		.FULL(),
		.RDCOUNT(),
		.RDERR(),
		.SBITERR(),
		.WRCOUNT(),
		.WRERR(),
		.DI(data[127:64]),
		.DIP(8'hFFFFFFFF),
		.RDCLK(clk),
		.RDEN(rd_en),
		.RST(reset),
		.WRCLK(clk),
		.WREN(wr_en)
   	);
   	
	FIFO36_72 #(
		.SIM_MODE("FAST"),
		.ALMOST_FULL_OFFSET(9'd300), // > Ethernet MAX length / 6 = 1516Byte/6 = 252
		.ALMOST_EMPTY_OFFSET(9'hA),
		.DO_REG(1),
		.EN_ECC_READ("FALSE"),
		.EN_ECC_WRITE("FALSE"),
		.EN_SYN("FALSE"),
		.FIRST_WORD_FALL_THROUGH("TRUE")
   	) rx_1_fifo (
		.ALMOSTEMPTY(),
		.ALMOSTFULL(),
		.DBITERR(),
		.DO(rx),
		.DOP(),
		.ECCPARITY(),
		.EMPTY(fifo_rx_empty),
		.FULL(),
		.RDCOUNT(),
		.RDERR(),
		.SBITERR(),
		.WRCOUNT(),
		.WRERR(),
		.DI(data[63:0]),
		.DIP(8'hFFFFFFFF),
		.RDCLK(clk),
		.RDEN(rd_en),
		.RST(reset),
		.WRCLK(clk),
		.WREN(wr_en)
   	);   	
   	
   	
   	

	
endmodule	