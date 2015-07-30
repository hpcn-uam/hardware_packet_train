/*******************************************************************************
 *
 *
 *  File:
 *        packet_generator.cpp
 *
 *
 *  Module:
 *        Packet_Generator
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
 
#include "packet.h"

using namespace std;

const ap_uint<4> version=4;
const ap_uint<4> hdr_len=5;
const ap_uint<13> fragment_offset=0;
const ap_uint<8> protocol=0x11;
const ap_uint<16> type=0x800;

#define fixed_paylod 18
#define udp_length    8
#define IP_HEADER	 20
#define fixed_lenght 42
#define MAC_HEADER 14


void packet_generator
(
		stream <axi_interface_type> &out_data,
		stream <axi_interface_type>	&out_control,
		ap_uint<32> *mac_destination1,
		ap_uint<32> *mac_destination2,
		ap_uint<32> *mac_source1,
		ap_uint<32> *mac_source2,
		ap_uint<8>	*diff_serv,
		ap_uint<16> *data_length, // size of packet
		ap_uint<16> *identification,
		ap_uint<8>	*ip_flags,
		ap_uint<8>	*time_to_live,
		ap_uint<32> *ip_destination,
		ap_uint<32> *ip_source,
		ap_uint<16> *source_port,
		ap_uint<16> *destination_port,
		ap_uint<32> *Stimestamp,
		ap_uint<32> *nStimestamp,
		ap_uint<32> *num_packet,
		ap_uint<32>	*packet_send,
		ap_uint<32> *time_between_packet,
		ap_uint<1>  *indefinitely
)
{
#pragma HLS INTERFACE ap_fifo port=out_control

#pragma HLS INTERFACE ap_fifo port=out_data


#pragma HLS RESOURCE variable=out_data core=AXIS metadata="-bus_bundle M_AXIS_RXD"
#pragma HLS RESOURCE variable=out_control core=AXIS metadata="-bus_bundle M_AXIS_RXS"


#pragma HLS RESOURCE variable=mac_destination1 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=mac_destination2 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=mac_source1 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=mac_source2 core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=diff_serv core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=data_length core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=identification core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=ip_flags core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

#pragma HLS RESOURCE variable=time_to_live core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=ip_destination core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=ip_source core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=source_port core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=destination_port core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

#pragma HLS RESOURCE variable=num_packet core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=packet_send core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=time_between_packet core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

#pragma HLS RESOURCE variable=indefinitely core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

#pragma HLS INTERFACE ap_none port=nStimestamp
#pragma HLS INTERFACE ap_none port=Stimestamp



#pragma HLS RESOURCE variable=return core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"

	int i,j,l,n;

	ap_uint<32> chk_sum,aux_chk_sum;

	axi_interface_type aux;
	int aux_length,valid_byte,extra_bytes,data_length_aux;
	ap_uint<32> invert;
	int16_t var;
	ap_uint<16> total_length;
	ap_uint<16> length,_identification;
	ap_uint<32> stimes,nstimes;

	ap_uint<32> padding=0;


	if (*data_length<60)
		data_length_aux=60;
	else
		data_length_aux= *data_length;

	total_length= data_length_aux-MAC_HEADER; // length of IP header

	length= data_length_aux-MAC_HEADER-IP_HEADER; // length of UDP header

	extra_bytes= data_length_aux - 60;

	//length=*data_length+fixed_paylod+udp_length;  // length of UDP header
	//total_length=length+ip_header; // length of IP header



	_identification =*identification,



	aux_chk_sum =(version.range(3,0),hdr_len.range(3,0),diff_serv->range(7,0))+
			total_length.range(15,0)+
			(ip_flags->range(2,0),fragment_offset.range(12,0))+
			(time_to_live->range(7,0),protocol.range(7,0))+
			ip_destination->range(31,16)+ip_destination->range(15,0)+
			ip_source->range(31,16)+ip_source->range(15,0) ;

	valid_byte = (extra_bytes%4);
	aux_length = (int) ((extra_bytes/4));
	if (valid_byte!=0){
		aux_length++;
	}


	do {

	num_pkt:	for (l=0;l<(uint32_t)*num_packet;l++){
//#pragma HLS PIPELINE rewind

#pragma HLS LOOP_TRIPCOUNT min=1 max=65536

			aux.keep=0xF;
			aux.last=0;

			gen_crtl: for (j=0;j<6;j++){

//#pragma HLS PIPELINE
				if (j==0){
					aux.data=0xA0000000;
				}
				else{
					aux.data=0;
				}
				if (j==5){
					aux.last=1;
				}
				/*else if (j==6){
					aux.last=0;
					aux.keep=0;
				}*/
				out_control.write(aux);
			}

			aux.keep=0xF;
			nstimes = *nStimestamp;
			stimes = *Stimestamp;  // save de timestamp at the time it begins to build the package

			gen_pkt: for (j=0; j < (((fixed_lenght+fixed_paylod)/4) +aux_length) ; j++) { // minimum is 15 but put on 14 for the last this crc
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=16 max=1000
				if (/*j<14*/j<13){
				switch (j){
					case 0:
						aux.data=(uint32_t)(mac_destination1->range(7,0),mac_destination1->range(15,8), // MAC DESTINATION
								mac_destination1->range(23,16),mac_destination1->range(31,24));
						break;
					case 1:
						aux.data= (uint32_t)(mac_source1->range(7,0),mac_source1->range(15,8), // MAC DESTINATION + MAC SOURCE
								mac_destination2->range(7,0),mac_destination2->range(15,8));
						break;
					case 2:
						aux.data=(uint32_t)(mac_source2->range(7,0),mac_source2->range(15,8), // MAC SOURCE
								mac_source2->range(23,16),mac_source2->range(31,24));
						break; // mac source 31:0
					case 3:
						aux.data=(uint32_t)(diff_serv->range(7,0),version.range(3,0), // TYPE + VERSION + HDR LEN + DIFF SERVICES
								hdr_len.range(3,0),type.range(7,0),type(15,8));
						break;
					case 4:
						aux.data=(uint32_t)(_identification.range(7,0),_identification.range(15,8), // TOTAL LENGTH + IDENTIFICATION
								total_length.range(7,0),total_length.range(15,8));
						chk_sum=aux_chk_sum+_identification.range(15,0);
						break;
					case 5:
						aux.data=(uint32_t)(protocol.range(7,0),time_to_live->range(7,0),fragment_offset.range(7,0), // IP FLAGS + FRAGMENT OFFSET + TIME TO LIVE + PROTOCOL
								ip_flags->range(2,0),fragment_offset.range(12,8));
						break;
					case 6:
						chk_sum=~(chk_sum.range(31,16)+chk_sum.range(15,0)); // HEADER CHECK SUM + IP SOURCE
						aux.data=(uint32_t)(ip_source->range(23,16),ip_source->range(31,24),
								chk_sum.range(7,0),chk_sum.range(15,8));
						break;
					case 7:
						aux.data=(uint32_t)(ip_destination->range(23,16),ip_destination->range(31,24), // IP SOURCE + IP DESTINATION
								ip_source->range(7,0),ip_source->range(15,8));
						break;
					case 8:
						aux.data=(uint32_t)(source_port->range(7,0),source_port->range(15,8), // IP DESTINATION + SOURCE PORT
								ip_destination->range(7,0),ip_destination->range(15,8));
						break;
					case 9:
						aux.data=(uint32_t)(length.range(7,0),length.range(15,8), // DESTINATION PORT + LENGHT
								destination_port->range(7,0),destination_port->range(15,8));
						break;
					case 10:
						aux.data=0x01000000 ; // CHECKSUM + Nº TIMESTAMP
						break;
					case 11:
						aux.data=(uint32_t)(stimes.range(7,0),stimes.range(15,8),
								stimes.range(23,16),stimes.range(31,24));        // SEG TIMESTAMP TRANSMIT
						break;
					case 12:
						aux.data=(uint32_t)(nstimes.range(7,0),nstimes.range(15,8),
								nstimes.range(23,16),nstimes.range(31,24)); // NANOSEG TIMESTAMP TRANSMIT
						break;
//					case 13:
//						aux.data=0;
//						break;
					}
				}
				else {
					aux.data=(uint32_t)(padding.range(7,0),padding.range(15,8), // PADDING
							padding.range(23,16),padding.range(31,24));
					padding=padding+1;
					if (j==((((fixed_lenght+fixed_paylod)/4) +aux_length-1))){
						aux.last=1;
						switch(valid_byte){
							case 1:
								aux.keep=0x1;
								break;
							case 2:
								aux.keep=0x3;
								break;
							case 3:
								aux.keep=0x7;
								break;
							default:
								break;
						}
					}
				}
			out_data.write(aux);

			}

			/*aux.data=0;
			aux.last=0;
			aux.keep=0;
			out_data.write(aux);*/
			_identification++;
	delay:		for (n=0;n<*time_between_packet;n++){
//#pragma HLS INLINE

#pragma HLS LOOP_TRIPCOUNT min=0 max=4294967295
				++aux.data;
				//out_data.write(aux);
			}
		}

		*packet_send=(uint32_t) l;

	} while (*indefinitely==1);

}
