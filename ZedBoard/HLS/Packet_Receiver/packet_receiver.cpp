/*******************************************************************************
 *
 *
 *  File:
 *        packet_receiver.cpp
 *
 *
 *  Module:
 *        Packet_Receiver
 *
 *  Author:
 *        Mario Ruiz
 *
 *
 *  Copyright (C) 2015 - Mario Ruiz and HPCN-UAM High Performance Computing and Networking
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
 
#include "receiver.h"


ap_uint<32> _received_packets=0;
ap_uint<32> _untagged_packets=0;
ap_uint<32> _market_packets=0;

ap_uint<32> memory[20480]; // memory for save lenght, identificator, and timestamp of the burst // max 4096 packet
ap_uint<32> address=0;
ap_uint<16> identificator=0;


void packet_receiver(
		stream <axi_interface_type> &in_data,
		stream <axi_interface_type> &in_control,
		stream <axi_interface_type> &out_data,
		stream <axi_interface_type> &out_control,
		ap_uint<32> *mac_destination1,
		ap_uint<32> *mac_destination2,
		ap_uint<32> *mac_source1,
		ap_uint<32> *mac_source2,
		ap_uint<32> *ip_destination,
		ap_uint<32> *ip_source,
		ap_uint<16> *source_port,
		ap_uint<16> *destination_port,
		ap_uint<32> *Stimestamp,
		ap_uint<32> *nStimestamp,
		ap_uint<32> *received_packets,
		ap_uint<32> *untagged_packets,
		ap_uint<32> *market_packets,
		ap_uint<1>	*analisys_mac,
		ap_uint<1>  *reset_values,
		ap_uint<32> *pos_read,
		ap_uint<32> *element_value
		){

#pragma HLS INTERFACE ap_fifo port=in_control
#pragma HLS INTERFACE ap_fifo port=in_data

#pragma HLS RESOURCE variable=in_data core=AXIS metadata="-bus_bundle S_AXIS_RXD"
#pragma HLS RESOURCE variable=in_control core=AXIS metadata="-bus_bundle S_AXIS_RXS"

#pragma HLS RESOURCE variable=out_data core=AXIS metadata="-bus_bundle M_AXIS_RXD"
#pragma HLS RESOURCE variable=out_control core=AXIS metadata="-bus_bundle M_AXIS_RXS"

#pragma HLS RESOURCE variable=mac_destination1 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=mac_destination2 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=mac_source1 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=mac_source2 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

#pragma HLS RESOURCE variable=ip_destination core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=ip_source core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=source_port core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=destination_port core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

#pragma HLS RESOURCE variable=received_packets core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=untagged_packets core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=market_packets core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

#pragma HLS RESOURCE variable=analisys_mac core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"


#pragma HLS RESOURCE variable=reset_values core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=pos_read core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=element_value core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"


#pragma HLS INTERFACE ap_none port=nStimestamp
#pragma HLS INTERFACE ap_none port=Stimestamp

#pragma HLS RESOURCE variable=return core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

	axi_interface_type aux,aux2,aux3;

	int j=0,acept;

	ap_uint<16> length;


	ap_uint<32> type=0x800;
	ap_uint<32> aux_data;
	ap_uint<32> stimes,nstimes;

	ap_uint<16> number_timestamp;



	aux.last=0;
	aux.keep=0xF;


	*element_value= memory[*pos_read]; // position of read

	acept=0; // start, to see if the packet is accepted


	if (*reset_values==1 && in_data.empty()){
		_received_packets = 0;
		_untagged_packets = 0;
		_market_packets   = 0;
		identificator = 0;
		address=0;
	}


	read_data:	while (!in_data.empty()){
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=1 max=1518 // only estimating clock cycles in simulation


		if (!in_control.empty()){			// read control and transmit control
			in_control.read(aux2);
			out_control.write(aux2);
		}


		in_data.read(aux);					// read data

		if (j!=10 && j!=14 && j!=13){

			aux_data = (uint32_t) aux.data;		// save data for analysis

			switch (j){
				case 0:
					nstimes = *nStimestamp;
					stimes = *Stimestamp;  // save de timestamp at the time it begins the package

					if ((((uint32_t)(mac_destination1->range(7,0),mac_destination1->range(15,8),
					mac_destination1->range(23,16),mac_destination1->range(31,24))) != (aux_data))&&
					(*analisys_mac==1)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 1:
					if ((((uint32_t)(mac_source1->range(7,0),mac_source1->range(15,8),
							mac_destination2->range(7,0),mac_destination2->range(15,8)))!=(aux_data))&&
							(*analisys_mac==1)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 2:
					if ((((uint32_t)(mac_source2->range(7,0),mac_source2->range(15,8),
							mac_source2->range(23,16),mac_source2->range(31,24)))!=(aux_data))&&
							(*analisys_mac==1)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 3:
					if (((uint32_t)(type.range(7,0),type.range(15,8)))!= aux_data.range(15,0)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 4:
					length = (aux_data.range(7,0),aux_data.range(15,8)); // really length
					memory[address++] = (length+14,identificator++); // save de lenght and identificator
					break;
				case 5:
					if ((uint16_t)0x11!=aux_data.range(31,24)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 6:
					if(((uint16_t)(ip_source->range(23,16),ip_source->range(31,24)
							))!= aux_data.range(31,16)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 7:
					if ((uint32_t)(ip_destination->range(23,16),ip_destination->range(31,24),
							ip_source->range(7,0),ip_source->range(15,8))!=(aux_data)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 8:
					if ((uint32_t)(source_port->range(7,0),source_port->range(15,8),
							ip_destination->range(7,0),ip_destination->range(15,8))!=(aux_data)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 9:
					if ((uint16_t)(destination_port->range(7,0),destination_port->range(15,8))!=
							aux_data.range(15,0)){
						acept=0;
					}
					else {
						acept++;
					}
					break;
				case 11:
					memory[address++]=(aux_data.range(7,0),aux_data.range(15,8),
							aux_data.range(23,16),aux_data.range(31,24));  // save the second of timestamp
					break;

				case 12:
					memory[address++]=(aux_data.range(7,0),aux_data.range(15,8),
							aux_data.range(23,16),aux_data.range(31,24)); // save the nanosecond of timestamp
					break;
			}
		}
		else if ((j==10 || j==14 || j==13) && acept==9) { // include received timestamp, I do this to avoid writing twice in the same memory

			if (j==10){
				number_timestamp = (aux.data.range(23,16),aux.data.range(31,24))+1;
				aux_data = (number_timestamp.range(7,0),number_timestamp.range(15,8)
						,aux.data.range(15,0));
			}
			else if (j==13){
				aux_data=(stimes.range(7,0),stimes.range(15,8),
						stimes.range(23,16),stimes.range(31,24));      // SEG TIMESTAMP RECEIVE
				memory[address++]=stimes;// SEG TIMESTAMP RECEIVE
//				cout << "Sec: " << hex << stimes << endl;
			} else if (j==14){
				aux_data=(nstimes.range(7,0),nstimes.range(15,8),
						nstimes.range(23,16),nstimes.range(31,24));   // NANOSEG TIMESTAMP RECEIVE
				memory[address++]=nstimes;   // NANOSEG TIMESTAMP RECEIVE
//				cout << "NSec: " << hex << nstimes << endl;
				_market_packets++;
				_received_packets++;
			}



		}
		else if (j==14 && acept!=9){
			aux_data = (uint32_t) aux.data;
			_untagged_packets++;
			_received_packets++;
			address-=3;
		}
		else {
			aux_data = (uint32_t) aux.data;
		}

		aux3.data= aux_data;
		aux3.keep=aux.keep;
		aux3.last=aux.last;

		out_data.write(aux3);

		j++;
	}


	cout << "Address  : " << dec << address << endl;



	*received_packets=_received_packets; // out data
	*untagged_packets=_untagged_packets;
	*market_packets=_market_packets;

}
