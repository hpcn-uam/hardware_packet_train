/*******************************************************************************
 *
 *  NetFPGA-10G http://www.netfpga.org
 *
 *  File:
 *        rater.v
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
// Create Date:    17:53:27 06/20/2014 
// Design Name: 
// Module Name:    rater 
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
module rater(
    input [31:0] limit,
    input clk,
    input reset,
    output reg generate_pulse,
	 output reg [31:0]generated_packets,
	 input my_reset
    );


reg [31:0] counter;
//reg [31:0]generated_packets ;
//reg [31:0] generate_pulse;



always @(posedge clk)
begin

	if(reset || my_reset)
	begin
		
			counter = 0;
			generated_packets =0;
	end
	else
	begin
			if(counter == limit )
			begin
				generate_pulse =1;
				counter =0;
				generated_packets = generated_packets +1;
			end
			else if(counter > limit)
			begin
				generate_pulse =0;
				counter =0;
			end
			else if(counter <limit)
			begin
					generate_pulse=0;
			end
						counter = counter +1;
	end

end //end always



endmodule
