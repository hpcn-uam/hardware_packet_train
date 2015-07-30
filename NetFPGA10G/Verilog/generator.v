/*******************************************************************************
 *
 *  NetFPGA-10G http://www.netfpga.org
 *
 *  File:
 *        generator.v
 *
 *  Library:
 *        hw/osnt/pcores/osnt_10g_interface_v1_11_a
 *
 *  Module:
 *        packet_generator
 *
 *  Author:
 *        Antonis Gavaletakis
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
`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    18:19:39 06/20/2014 
// Design Name: 
// Module Name:    generator 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module generator
#(


 //Master AXI Stream Data Width
    parameter C_M_AXIS_DATA_WIDTH=256,
    parameter C_S_AXIS_DATA_WIDTH=256,
    parameter C_M_AXIS_TUSER_WIDTH=128,
    parameter C_S_AXIS_TUSER_WIDTH=128,
    parameter SRC_PORT_POS=16,
    parameter DST_PORT_POS=24,
    parameter HEADER_LENGTH= 400,



	// Master AXI Stream Data Width
	parameter	C_FAMILY						= "virtex5",
	parameter	C_S_AXI_DATA_WIDTH		= 32,
	parameter	C_S_AXI_ADDR_WIDTH		= 32,
	parameter	C_USE_WSTRB					= 0,
	parameter	C_DPHASE_TIMEOUT			= 0,
	parameter	C_BASEADDR					= 32'hFFFFFFFF,
	parameter	C_HIGHADDR					= 32'h00000000,
	parameter	C_S_AXI_ACLK_FREQ_HZ		= 100
)
(

    // Global Ports
    input axi_aclk,
    input axi_resetn,
     

    // Master Stream Ports (interface to data path)
    output reg [C_M_AXIS_DATA_WIDTH - 1:0] m_axis_tdata,
    output reg [((C_M_AXIS_DATA_WIDTH / 8)) - 1:0] m_axis_tstrb,
    output reg [C_M_AXIS_TUSER_WIDTH-1:0] m_axis_tuser,
    output reg m_axis_tvalid,
    input  m_axis_tready,
    output reg  m_axis_tlast,
	 
	 // REGISTERS INFORMATION
	 input [31:0]		num_packets,			// number of packets to generete
	 input [31:0]		payload_size,			// payload size in bytes
	 input		 		user_register_write, // it is a pulse .! indiacates the start!
	 input 				generate_pulse, 		// comes from RATER!
	 
	 
	 // headers_fields
	// input [31:0] ip_length,
	 input [31:0] ip_srcip,
	 input [31:0] ip_dstip,
	// input [31:0] ip_checksum,
	 
	 input [48:0] mac_dstaddress,
	 input [48:0] mac_srcaddress,
	 
	 input [15:0] udp_srcport,
	 input [15:0] udp_dstport,
	// input [15:0] udp_length,
	 
	 input [63:0] time_stamp,
	 
	 output reg [31:0] packet_generated,
	 output reg [31:0] packet_left,
	 output reg [31:0] state,
	 output reg [31:0] new_packet_generated,
	 
	 input my_reset,
	 input my_start
	 
	 
	 );
	 
	 
	 //Local Registers
	 wire [15:0]					packet_size;
	 wire [HEADER_LENGTH -1 : 0]			headers;
	 wire 						start;
	 reg 						need_more_packet;
	 wire	[15:0]					header_size;
	 
	 
		 
	 
	
	 reg	[0:111] 				mac_header; 	//14  bytes
	 reg	[0:159]					ip_header;		//20 bytes
	 reg	[0:63]					udp_header;		//8 bytes
	 reg 	[0:95]					rtp_header;		//12 bytes			
	
	
	 reg 	[2:0]                next_state;//, state;
	 reg						snd_payload;
	 reg						end_of_packet;
	 reg	[31:0]					snd_data;
	// reg  [31:0]					packet_left;
	reg	[0:15] 						sum  ;
	reg	[0:18] 						pros	;
	
	 assign header_size  = 14+20+8+8; // mac + ip + udp+ timestamp
	 assign start		= generate_pulse & need_more_packet;// & need_more_packet;		//start to send packets
	 assign headers 	= {mac_header[0:111], ip_header[0:159], udp_header[0:63],time_stamp[63:0] };
	 assign packet_size	= header_size+payload_size; // +8 timestamp

	//Local Parameters
	parameter WAIT 			= 0;
   parameter PREPARE 		= 1;
   parameter SNT_HEADER 	= 2;
   parameter SNT_HEADER_PAYLOAD =3;
   parameter SNT_PAYLOAD 	= 4;
	parameter END_PACKET = 5;
	 
	integer i; 
	
	integer j;
	integer k;
	 //

always @ (posedge axi_aclk) begin
		if(!axi_resetn || my_reset)begin
			packet_generated <=  0;
		end
		else if(user_register_write)
			packet_generated <= 0;
		else begin
			if (m_axis_tlast && m_axis_tready && m_axis_tvalid  )
			packet_generated <=  packet_generated +1;	 
		end
end



always@ (posedge axi_aclk) begin
		if(!axi_resetn|| my_reset)begin
			state <=  WAIT;
		end
		else begin
			state <=  next_state;	 
		end
end
	 
	 //-----------------------


always@(*) 
begin
	      next_state= state;
	      case(state)
	      WAIT:begin
				if(start)
					next_state = PREPARE;
				else
					next_state = WAIT;
			end

	      PREPARE:begin
		      next_state = SNT_HEADER;
		      end
				
	      SNT_HEADER:begin
					if(m_axis_tready)
						next_state = SNT_HEADER_PAYLOAD;
					else
						next_state = SNT_HEADER;
	      end
			
	      SNT_HEADER_PAYLOAD:begin
					if(m_axis_tready)
						if(snd_payload)
							next_state = SNT_PAYLOAD;
						else
							next_state = END_PACKET;
					else
						next_state = SNT_HEADER_PAYLOAD;
	      end
	      SNT_PAYLOAD:begin
				if(end_of_packet)
					next_state=END_PACKET;
				else
					next_state = SNT_PAYLOAD;
	      end
			END_PACKET:begin
				next_state = WAIT;
			end
			
			
	      endcase
end		//END  ALWAYS


always@(posedge axi_aclk)
begin

	if(!axi_resetn || my_reset)
	begin
		snd_payload   = 0;
		snd_data    = 0 ;
		end_of_packet = 0;
		m_axis_tdata  = 0;
		m_axis_tstrb  = 0;
		m_axis_tuser  = 0;
		m_axis_tlast  = 0;
		m_axis_tvalid = 0;
		
		ip_header  = 0;
		mac_header = 0;
		udp_header = 0;
		packet_left =0;
		need_more_packet =0;
		new_packet_generated =0; 
		
		sum  ='b0;
		pros ='b0;
	end
	else
	begin
		
		case(state)
		WAIT:begin
			snd_payload = 0 ;
			snd_data    = 0 ;
			end_of_packet = 0;
			m_axis_tdata  = 'hDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF;
			m_axis_tstrb  = 'hDEADBEEFDEADBEEFDEADBEEFDEADBEEF;
			m_axis_tuser  = 0;
			m_axis_tlast  = 0;
			m_axis_tvalid = 0;
			
			ip_header  = 0;
			mac_header = 0;
			udp_header = 0;
			
			pros ='b0;
		
			
		//------------------------
			if(num_packets > packet_generated)
				need_more_packet =1;
			else
				need_more_packet =0;
		//------------------------
		end
		PREPARE:begin
			$display ("\npacket : %d \n",packet_left );
			ip_header [0:3] 		= 4'd4;	//VERSION
			ip_header [4:7] 		= 4'd5; //	IHL internet header length
			ip_header [8:15]	   = 3'd0;   //Differentiated Services // useless field
			ip_header [16:31] 	= 28+ payload_size +8;//ip_length; // total length se bytes // 
			ip_header [32:47]   	= packet_generated[15:0]; //16'd0; // Identification ???????
			ip_header [48:50] 	= 3'b000; //flags // isws 100 ? 
			ip_header [51:63]	   = 13'd0;//Fragment Offset
			ip_header[64:71]	   = 8'd255; //TTL
			ip_header[72:79]	   = 8'd17;// protocol UDP ? 
			//ip_header[80:95]	   = 16'd0;//Header checksum
			ip_header[96:127]	   =ip_srcip;//32'b11000000101010000000000100000001;// srcip;//source_address;
			ip_header[128:159] 	=ip_dstip;//32'b11000000101010000000000100000001;//dstip; // destination_address;
		
		

			pros[0:18]=  	((ip_header[0:15]+ip_header[16:31])+
					(ip_header[32:47]+ip_header[48:63]))+
					((ip_header[49:79]+ip_header[80:95])+
					(ip_header[96:111]+ip_header[112:127]))+
					(ip_header[128:143]+ip_header[144:159]);



			sum[0:15] = pros[3:18]+ pros[0:2];	//0xC78A
			ip_header[80:95]= 'hFFFF- sum[0:15];//Header checksum 0x3875
		//---
			mac_header[0:47] 		= mac_dstaddress;// 48'hffffffffffff;//destination_address;		
			mac_header[48:95]		= mac_srcaddress;//48'haabbccddeeff;//source_address;
			mac_header[96:111]	= 16'h0800;	// TYPE 0x0800
		//---

			udp_header [0:15] 	= udp_srcport;//20000;//srcport; //source_port;
			udp_header [16:31] 	= udp_dstport;//20000;//dstport; //destination_port;
			udp_header [32:47] 	= 8+payload_size +8; //Length
			udp_header [48:63]	= 16'd0; //checksum .. i ola mides.. i upologizw swsta. ??? 
			
			//DIMIOURGIA TOY rtp header
			/*rtp_header[0:1]='d2; // version always assign to 2
			rtp_header[2]	=0; // padding
			rtp_header[3]	=0; // extension
			rtp_header[4:7]	=0; // csrc count
			rtp_header[8]	=0; //marker
			
			rtp_header[9:15]=0;	   //G.711 ????
			
			
			rtp_header[16:31]	=rtp_sq; // sequence number
			rtp_header[32:63]	=time_stamp; // timestamp
			rtp_header[64:95]	=32'hAD0F01AD; //SSRC
			*/
			end
		
		SNT_HEADER:begin
			if (m_axis_tready)
			begin
			
			
					for(k =0 ; k < 32; k= k+1)
						for(j=0; j<8; j= j+1)
							m_axis_tdata[248-(k*8)+j]  = headers [(k*8)+j+144 ];
							
							
					m_axis_tstrb  = 'hFFFFFFFFFFFF;
					m_axis_tuser  = { {96{1'b0}}, 8'hff,8'b0,packet_size};
					m_axis_tvalid = 1;
					m_axis_tlast  = 0;
					$display ("snd data : %h \n",m_axis_tdata );
					
					if(( (packet_size*8) - C_M_AXIS_DATA_WIDTH ) > C_M_AXIS_DATA_WIDTH)
						begin
							snd_payload =1;
						end
			end
			else
			begin
					m_axis_tdata  = 0;
					m_axis_tstrb  = 'h0;
					m_axis_tuser  = 0;
					m_axis_tvalid = 0;
					m_axis_tlast  = 0;
			
			end
		
		end
		SNT_HEADER_PAYLOAD:begin
			if (m_axis_tready)
			begin
					if(( (packet_size*8) - C_M_AXIS_DATA_WIDTH ) > C_M_AXIS_DATA_WIDTH)
					begin
						snd_payload =1;
						//m_axis_tdata  = {{(2*C_M_AXIS_DATA_WIDTH - HEADER_LENGTH){1'b1}} , headers[ HEADER_LENGTH -1 : C_M_AXIS_DATA_WIDTH ]}; // edw
						
						for(k =0 ; k < 32; k= k+1)
							for(j=0; j<8; j= j+1)
								if ((k*8+j)>111)
									m_axis_tdata[248-(k*8)+j]  = headers [(k*8)+j-112];
								else
									m_axis_tdata[248-(k*8)+j]  = 1;
						
						
						m_axis_tstrb  = 'hFFFFFFFFFFFF;
						m_axis_tuser  = { {96{1'b0}}, 8'hff,8'b0,packet_size};
						m_axis_tlast  = 0;
						m_axis_tvalid = 1;
						$display ("snd data : %h \n",m_axis_tdata );
						
						snd_data = C_M_AXIS_DATA_WIDTH * 2 ;
					end	
					else
					begin
						snd_payload = 0; 
						snd_data = C_M_AXIS_DATA_WIDTH + (packet_size  - 32 )*8 ;
						//m_axis_tdata  = {{(2*C_M_AXIS_DATA_WIDTH- HEADER_LENGTH){1'b1}}, headers[HEADER_LENGTH -1 : C_M_AXIS_DATA_WIDTH ]}  ;
						
						for(k =0 ; k < 32; k= k+1)
							for(j=0; j<8; j= j+1)
								if ((k*8+j)>111)
									m_axis_tdata[248-(k*8)+j]  = headers [(k*8)+j-112];
								else
									m_axis_tdata[248-(k*8)+j]  = 1;
						
						
						//$display ("snt_header payload\n");
						//$display ("lala : %d \n" ,packet_size  - 42);
						for(i=0; i<(C_M_AXIS_DATA_WIDTH / 8); i= i+1)begin
							if(i < (packet_size  - 32))
								m_axis_tstrb [i] =1;
							else
								m_axis_tstrb [i] = 0;
						end
						m_axis_tuser  = { {96{1'b0}}, 8'hff,8'b0,packet_size};
						m_axis_tlast  = 1;
						m_axis_tvalid = 1;
						$display ("snd data : %h \n",m_axis_tdata );
					end
					
					
					
					
							//$display ("quedan : %d" , ( (packet_size *8) - (snd_data + C_M_AXIS_DATA_WIDTH) ));
							if( 768   > (packet_size *8))
							begin
								end_of_packet = 1;
								$display (" END OF PACKET IS TRUE");
								
							end
						
						
						
						
						
				end // end of if (m_axis_tready)
				else
				begin
						m_axis_tdata  = 0;
						m_axis_tstrb  = 'h0;
						m_axis_tuser  = 0;
						m_axis_tvalid = 0;
						m_axis_tlast  = 0;
				
				end
	   end
		SNT_PAYLOAD:begin
			if (m_axis_tready)
			begin
				
						
						
						
						
						
						if(( (packet_size *8) - (snd_data) ) > C_M_AXIS_DATA_WIDTH)
						begin
						
							m_axis_tdata  = {C_M_AXIS_DATA_WIDTH/4  {4'b1011}} ;
							m_axis_tstrb  = 'hFFFFFFFFFFFF;
							m_axis_tuser  = { {96{1'b0}}, 8'hff,8'b0,packet_size};
							m_axis_tlast  = 0;
							m_axis_tvalid = 1;
							$display ("snd data : %h \n",m_axis_tdata );
							
							//end_of_packet = 0;
							snd_data = snd_data + C_M_AXIS_DATA_WIDTH;
						//	$display ("snd_data: %d" ,  snd_data );
						
							
							if(( (snd_data +C_M_AXIS_DATA_WIDTH)     ) >= (packet_size *8))
							begin
								end_of_packet = 1;
							end
						end			
						else
						begin
						//end_of_packet =1;
						//$display ("SEND_ PAYLOAD_ QUEDAN  : %d \n" ,(packet_size  - snd_data /8));
						
						
							m_axis_tdata  = {C_M_AXIS_DATA_WIDTH/4  {4'b1001}};
							
							for(i=0; i<(C_M_AXIS_DATA_WIDTH / 8); i= i+1)begin
								if(i < (packet_size  - snd_data /8))
									m_axis_tstrb [i] =1;
								else
									m_axis_tstrb [i] = 0;
							end
							m_axis_tuser  = { {96{1'b0}}, 8'hff,8'b0,packet_size};
							m_axis_tlast  = 1;
							m_axis_tvalid = 1;
							$display ("snd data : %h \n",m_axis_tdata );
							
							
							snd_data = snd_data + (packet_size *8 - snd_data);
							
							if(( (snd_data+C_M_AXIS_DATA_WIDTH)      ) >= (packet_size *8))
							begin
								end_of_packet = 1;
							end
						
						end

			end // end of if m_axi_tready
			else
			begin
					m_axis_tdata  = 0;
					m_axis_tstrb  = 'h0;
					m_axis_tuser  = 0;
					m_axis_tvalid = 0;
					m_axis_tlast  = 0;
			
			end
		end
		END_PACKET:begin
				snd_payload   = 0;
				snd_data      = 0 ;
				end_of_packet = 0;
				m_axis_tdata  = 'hDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF;
				m_axis_tstrb  = 'hDEADBEEFDEADBEEFDEADBEEFDEADBEEF;
				m_axis_tuser  = 0;
				m_axis_tlast  = 0;
				m_axis_tvalid = 0;
		
				packet_left = packet_left -1;  
				new_packet_generated = new_packet_generated +1;
				
		end
		endcase
	
	end // end else
end // end always
	 
	 
endmodule
