/*******************************************************************************
 *
 *  NetFPGA-10G http://www.netfpga.org
 *
 *  File:
 *        packet_counter.v
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

module packet_counter
#(

   // TIMESTAMP_WIDTH
   parameter TIMESTAMP_WIDTH = 64,

 //Master AXI Stream Data Width
    parameter C_M_AXIS_DATA_WIDTH=256,
    parameter C_S_AXIS_DATA_WIDTH=256,
    parameter C_M_AXIS_TUSER_WIDTH=128,
    parameter C_S_AXIS_TUSER_WIDTH=128,
    parameter SRC_PORT_POS=16,
    parameter DST_PORT_POS=24,



	// Master AXI Stream Data Width
	parameter	C_FAMILY					= "virtex5",
	parameter	C_S_AXI_DATA_WIDTH		= 32,
	parameter	C_S_AXI_ADDR_WIDTH		= 32,
	parameter	C_USE_WSTRB					= 0,
	parameter	C_DPHASE_TIMEOUT			= 0,
	parameter	C_BASEADDR					= 32'hFFFFFFFF,
	parameter	C_HIGHADDR					= 32'h00000000,
	parameter	C_S_AXI_ACLK_FREQ_HZ		= 100
)
(


    input [TIMESTAMP_WIDTH-1:0] stamp_counter,
    // Global Ports
    input axi_aclk,
    input axi_resetn,
    

    // Master Stream Ports (interface to data path)
    output [C_M_AXIS_DATA_WIDTH - 1:0] m_axis_tdata,
    output [((C_M_AXIS_DATA_WIDTH / 8)) - 1:0] m_axis_tstrb,
    output [C_M_AXIS_TUSER_WIDTH-1:0] m_axis_tuser,
    output m_axis_tvalid,
    input  m_axis_tready,
    output m_axis_tlast,


	// Slave AXI Ports
	input				S_AXI_ACLK,
	input				S_AXI_ARESETN,
	input	[C_S_AXI_ADDR_WIDTH-1:0]	S_AXI_AWADDR,
	input					S_AXI_AWVALID,
	input	[C_S_AXI_DATA_WIDTH-1:0]	S_AXI_WDATA,
	input	[C_S_AXI_DATA_WIDTH/8-1:0]	S_AXI_WSTRB,
	input				S_AXI_WVALID,
	input				S_AXI_BREADY,
	input	[C_S_AXI_ADDR_WIDTH-1:0]	S_AXI_ARADDR,
	input				S_AXI_ARVALID,
	input				S_AXI_RREADY,
	output				S_AXI_ARREADY,
	output	[C_S_AXI_DATA_WIDTH-1:0]	S_AXI_RDATA,
	output	[1:0]			S_AXI_RRESP,
	output				S_AXI_RVALID,
	output				S_AXI_WREADY,
	output	[1:0]			S_AXI_BRESP,
	output				S_AXI_BVALID,
	output				S_AXI_AWREADY
);

///////////////////////////////////////////////////////////

	localparam	NUM_RW_REGS		= 13;
	localparam	NUM_RO_REGS		= 8;
//////////////////////////////////////////////////////////////
	wire	Bus2IP_Clk;
	wire	Bus2IP_Resetn;
	wire	[C_S_AXI_ADDR_WIDTH-1:0]	Bus2IP_Addr;
	wire	[0:0]				Bus2IP_CS;
	wire					Bus2IP_RNW;
	wire	[C_S_AXI_DATA_WIDTH-1:0]	Bus2IP_Data;
	wire	[C_S_AXI_DATA_WIDTH/8-1:0]	Bus2IP_BE;
	wire	[C_S_AXI_DATA_WIDTH-1:0]	IP2Bus_Data;
	wire					IP2Bus_RdAck;
	wire					IP2Bus_WrAck;
	wire					IP2Bus_Error;
		
	wire	[NUM_RW_REGS*C_S_AXI_DATA_WIDTH-1:0]	rw_regs;
	wire	[NUM_RO_REGS*C_S_AXI_DATA_WIDTH-1:0]	ro_regs;


	wire [31:0]generated_packets;
	


//////////////////////////////////////////
	wire [31:0] num_packets;
	wire [31:0] payload_size;
	wire user_register_write;
	
	wire [48:0] mac_dstaddress;
	wire [48:0] mac_srcaddress;

//	wire [31:0] ip_length;
	wire [31:0] ip_srcip;
	wire [31:0] ip_dstip;
//	wire [31:0] ip_checksum;

	wire [15:0] udp_srcport;
	wire [15:0] udp_dstport;
//	wire [15:0] udp_length;
	//wire [63:0] time_stamp;
  
	wire [31:0] limit;
	wire [31:0] packet_generated_gen;
	
	wire [31:0] packet_left;
	wire [31:0] state;
	wire [31:0] new_packet_generated;
	wire my_reset;
	wire my_start;

	
	wire [35:0] CONTROL0;

// assign mac_dstaddress = {rw_regs[63:32],rw_regs[15:0]};
// assign mac_srcaddress = {rw_regs[127:96],rw_regs[79:64]};
// 
// assign ip_length = rw_regs[159:128];
// assign ip_srcip = rw_regs[191:160];
// assign ip_dstip = rw_regs[223:192];
// assign ip_checksum= rw_regs[255:224];
// 
// assign udp_srcport= rw_regs[287:256];
// assign udp_dstport= rw_regs[319:288];
// assign udp_length= rw_regs[351:320];
// 
// assign time_stamp= {rw_regs[415:384],rw_regs[383:352]};
// 
// assign num_packets = rw_regs[447:416];
// assign payload_size = rw_regs[479:448];
// //assign user_register_write = ;
// 
// assign limit = rw_regs[511:480];
// assign my_reset = rw_regs[512];

//assign mac_dstaddress = 48'h06c626da70316;
//assign mac_srcaddress = 48'h06c626da70316;

//assign ip_length = 32'had; 		//USELESS
//assign ip_srcip = 32'h4137df2d;
//assign ip_dstip = 32'h4137df2e;
//assign ip_checksum= 32'h7a75;	//USELESS

//assign udp_srcport= 1'h9c59;
//assign udp_dstport= 16'h9c59;
//assign udp_length= 16'ha;

//assign time_stamp= 64'h4137df2d4137df2d;

//assign num_packets = 32'd1000;
//assign payload_size = 32'd50;
//assign user_register_write = ;




assign my_reset = rw_regs[0];
assign my_start = rw_regs[32];
assign limit = rw_regs[95:64];//32'hA;
assign payload_size = rw_regs[127:96];

 assign ip_srcip = rw_regs[159:128];
 assign ip_dstip = rw_regs[191:160];
 assign udp_srcport= rw_regs[223:192];
 assign udp_dstport= rw_regs[255:224];
 assign mac_dstaddress = {rw_regs[319:288],rw_regs[287:256]};
 assign mac_srcaddress = {rw_regs[383:352],rw_regs[351:320]};

 assign num_packets = rw_regs[415:384];



/*
assign ro_regs[31:0] = 32'h06c626da70316];
assign ro_regs[63:32] =rw_regs[63:32]; 
assign ro_regs[95:64]   = rw_regs[95:64];

assign ro_regs[127:96]  = rw_regs[127:96] ;
assign ro_regs[159:128] = rw_regs[159:128];
assign ro_regs[191:160] = rw_regs[191:160];
assign ro_regs[223:192] = rw_regs[223:192];
assign ro_regs[255:224] = rw_regs[255:224];
assign ro_regs[287:256] = rw_regs[287:256];
assign ro_regs[319:288] = rw_regs[319:288];
assign ro_regs[351:320] = rw_regs[351:320];
assign ro_regs[383:352] = rw_regs[383:352];
assign ro_regs[415:384] = rw_regs[415:384];
assign ro_regs[447:416] = rw_regs[447:416];
assign ro_regs[479:448] = rw_regs[479:448];
assign ro_regs[511:480] = rw_regs[511:480];


assign ro_regs[543:512] = generated_packets; //rater
assign ro_regs[575:544] = packet_generated_gen; // generator
assign ro_regs[607:576] = packet_left;
assign ro_regs[639:608] = state;
assign ro_regs[671:640] = new_packet_generated;
*/



assign ro_regs[31:0] = 32'h06c626da;
assign ro_regs[63:32] = generated_packets; //rater
assign ro_regs[95:64]   = packet_generated_gen; // generator

assign ro_regs[127:96]  = packet_left;
assign ro_regs[159:128] = state;
assign ro_regs[191:160] = new_packet_generated;
assign ro_regs[223:192] = stamp_counter[63:32];
assign ro_regs[255:224] = stamp_counter[31:0];



/*
always @(*)
begin

	if(S_AXI_ARADDR ==  C_BASEADDR+'h3C)
		user_register_write = S_AXI_WVALID;

	else
		user_register_write =0;

end
*/

wire signal_in;
wire edge_detected;
reg signal_d;

assign signal_in = my_start;

always @(posedge axi_aclk)
begin
	if(!axi_resetn)
		signal_d <=  1'b0;
	else
		signal_d <=  signal_in;
end

assign user_register_write = signal_in & (~signal_d);




//////////////////////////chip scope

/*

chipscope_icon ICON (
    .CONTROL0(CONTROL0) // INOUT BUS [35:0]
);

chipscope_ila ILA(
    .CONTROL(CONTROL0), // INOUT BUS [35:0]
    .CLK(axi_aclk), // IN
    .TRIG0(m_axis_tdata), // IN BUS [255:0]
    .TRIG1(m_axis_tstrb), // IN BUS [31:0]
    .TRIG2(m_axis_tuser), // IN BUS [127:0]
    .TRIG3(m_axis_tvalid), // IN BUS [0:0]
    .TRIG4(m_axis_tready), // IN BUS [0:0]
    .TRIG5(m_axis_tlast), // IN BUS [0:0]
    .TRIG6(new_packet_generated) // IN BUS [31:0]
);
*/
// -- AXILITE IPIF
axi_lite_ipif_1bar
#(
	.C_S_AXI_DATA_WIDTH	(C_S_AXI_DATA_WIDTH),
	.C_S_AXI_ADDR_WIDTH	(C_S_AXI_ADDR_WIDTH),
	.C_USE_WSTRB			(C_USE_WSTRB),
	.C_DPHASE_TIMEOUT		(C_DPHASE_TIMEOUT),
	.C_BAR0_BASEADDR		(C_BASEADDR),
	.C_BAR0_HIGHADDR		(C_HIGHADDR))
axi_lite_ipif_inst
(
	.S_AXI_ACLK		(S_AXI_ACLK),
	.S_AXI_ARESETN		(S_AXI_ARESETN),
	.S_AXI_AWADDR		(S_AXI_AWADDR),
	.S_AXI_AWVALID		(S_AXI_AWVALID),
	.S_AXI_WDATA		(S_AXI_WDATA),
	.S_AXI_WSTRB		(S_AXI_WSTRB),
	.S_AXI_WVALID		(S_AXI_WVALID),
	.S_AXI_BREADY		(S_AXI_BREADY),
	.S_AXI_ARADDR		(S_AXI_ARADDR),
	.S_AXI_ARVALID		(S_AXI_ARVALID),
	.S_AXI_RREADY		(S_AXI_RREADY),
	.S_AXI_ARREADY		(S_AXI_ARREADY),
	.S_AXI_RDATA		(S_AXI_RDATA),
	.S_AXI_RRESP		(S_AXI_RRESP),
	.S_AXI_RVALID		(S_AXI_RVALID),
	.S_AXI_WREADY		(S_AXI_WREADY),
	.S_AXI_BRESP		(S_AXI_BRESP),
	.S_AXI_BVALID		(S_AXI_BVALID),
	.S_AXI_AWREADY		(S_AXI_AWREADY),

	//	Controls	to	the	IP/IPIF	modules
	.Bus2IP_Clk		(Bus2IP_Clk),
	.Bus2IP_Resetn		(Bus2IP_Resetn),
	.Bus2IP_Addr		(Bus2IP_Addr),
	.Bus2IP_RNW		(Bus2IP_RNW),
	.Bus2IP_BE		(Bus2IP_BE),
	.Bus2IP_CS		(Bus2IP_CS),
	.Bus2IP_Data		(Bus2IP_Data),
	.IP2Bus_Data		(IP2Bus_Data),
	.IP2Bus_WrAck		(IP2Bus_WrAck),
	.IP2Bus_RdAck		(IP2Bus_RdAck),
	.IP2Bus_Error		(IP2Bus_Error));


// -- IPIF REGS
ipif_regs
#(
.C_S_AXI_DATA_WIDTH	(C_S_AXI_DATA_WIDTH	),	
.C_S_AXI_ADDR_WIDTH	(C_S_AXI_ADDR_WIDTH	),
.NUM_RW_REGS		(NUM_RW_REGS			),
.NUM_RO_REGS		(NUM_RO_REGS			))
ipif_regs_inst	(			
	.Bus2IP_Clk	(Bus2IP_Clk),
	.Bus2IP_Resetn	(Bus2IP_Resetn),	
	.Bus2IP_Addr	(Bus2IP_Addr),
	.Bus2IP_CS	(Bus2IP_CS[0]),
	.Bus2IP_RNW	(Bus2IP_RNW),
	.Bus2IP_Data	(Bus2IP_Data),
	.Bus2IP_BE	(Bus2IP_BE),
	.IP2Bus_Data	(IP2Bus_Data),
	.IP2Bus_RdAck	(IP2Bus_RdAck),
	.IP2Bus_WrAck	(IP2Bus_WrAck),
	.IP2Bus_Error	(IP2Bus_Error),

	.rw_regs	(rw_regs),
	.ro_regs	(ro_regs));





rater rater1 (
		.limit(limit), 
		.clk(axi_aclk), 
		
		.reset(!axi_resetn), 
		.generate_pulse(generate_pulse), 
		.generated_packets(generated_packets),
		.my_reset(my_reset)
	);
	
	
	
	// Instantiate the Unit Under Test (UUT)
	generator gen (
		.axi_aclk(axi_aclk), 
		.axi_resetn(axi_resetn), 
		
		.m_axis_tdata(m_axis_tdata), 
		.m_axis_tstrb(m_axis_tstrb), 
		.m_axis_tuser(m_axis_tuser), 
		.m_axis_tvalid(m_axis_tvalid), 
		.m_axis_tready(m_axis_tready), 
		.m_axis_tlast(m_axis_tlast), 
		
		.num_packets(num_packets), 
		.payload_size(payload_size), 
		.user_register_write(user_register_write), 
		.generate_pulse(generate_pulse), 
		
	//	.ip_length(ip_length), 
		.ip_srcip(ip_srcip), 
		.ip_dstip(ip_dstip), 
	//	.ip_checksum(ip_checksum), 
		
		.mac_dstaddress(mac_dstaddress), 
		.mac_srcaddress(mac_srcaddress), 
		
		.udp_srcport(udp_srcport), 
		.udp_dstport(udp_dstport), 
//		.udp_length(udp_length), 
		//.time_stamp(64'hAAAAAAAAAAAAAAAA),			
		.time_stamp(64'hFFFFFFFFFFFFFFFF),
		.packet_generated (packet_generated_gen),
		.packet_left (packet_left),
		.state(state),
		.new_packet_generated(new_packet_generated),
		
		.my_reset(my_reset),
		.my_start(my_start)
		
	);

endmodule
