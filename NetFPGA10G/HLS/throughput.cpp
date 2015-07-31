/*******************************************************************************
 *
 *
 *  File:
 *        throughput.cpp
 *
 *
 *  Module:
 *        Network Parameters Calculator
 *
 *  Author:
 *        Mario Ruiz, Antonis Gavaletakis, Gustavo Sutter
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

#include "throughput.h"


int throughput_calc (
		stream <axi_interface_type> &in_data,
		ap_uint<16> 				*num_packets,
		ap_uint<16>					*num_packets_jitter,
		ap_uint<16>					*num_packets_throughput,
		ap_uint<16>					*one_way_delay,
		ap_uint<16>					*jitter,
		ap_uint<16>					*length,
		ap_uint<34> 				*result,
		ap_uint<1>  				*end_calculations )
{

#pragma HLS RESOURCE variable=in_data 					core=AXIS metadata="-bus_bundle S_AXIS_RXD"
#pragma HLS RESOURCE variable=num_packets 				core=AXI4LiteS
#pragma HLS RESOURCE variable=num_packets_jitter 		core=AXI4LiteS
#pragma HLS RESOURCE variable=num_packets_throughput 	core=AXI4LiteS	
#pragma HLS RESOURCE variable=one_way_delay 			core=AXI4LiteS	
#pragma HLS RESOURCE variable=jitter 					core=AXI4LiteS
#pragma HLS RESOURCE variable=length 					core=AXI4LiteS
#pragma HLS RESOURCE variable=result 					core=AXI4LiteS
#pragma HLS RESOURCE variable=result 					core=AXI4LiteS	
#pragma HLS RESOURCE variable=end_calculations 			core=AXI4LiteS
#pragma HLS RESOURCE variable=return 					core=AXI4LiteS

	ap_uint<50> sum_thr =0;

	ap_uint<16> L ;

	ap_uint<34> throughput_instant;

	ap_uint<32> sec_dif;

	ap_uint<41> payload_mul;

	axi_interface_type input_buffer;


	struct timestamping ts_rx,ts_rx_1,ts_rx_2,ts_tx;
	ap_uint<16> ident, ident_1, ident_2;
	ap_uint<ETH_INTERFACE_WIDTH> input_data ;

	ap_uint<64> delay;
	ap_uint<32> sum_jitter=0;

	ap_int<64> jitter_instant;

	ap_uint<16> num_pack = 0,num_pkt_thr=0,num_pkt_jitter=0;

	ap_int<32> time_stamps_diff,time_stamps_diff_1;

	ap_uint<50> sum_delay=0;

	int end_of_stream =0;


	cout << "~~~~~~~~~~~~~~~~~~~~ New test ~~~~~~~~~~~~~~~~~~~~"<< endl<< endl << endl;

	the_loop: while (end_of_stream != 1) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=10 max=1000

		
		if (in_data.empty()){
			if (*end_calculations == 1 ){
				end_of_stream = 1;
				cout << "end of stream:"<< dec << end_of_stream<< endl;
				break;
			}
		}
		else {
			in_data.read(input_buffer);
			if (end_of_stream == 0) {//break this execution

				input_data = input_buffer.data; // read axi4-stream

				/* Save previous values */

				ts_rx_2 = ts_rx_1;
				ts_rx_1 = ts_rx;
				ident_2 = ident_1;
				ident_1 = ident;

				/* read new values */
				
				L				=	input_data.range(159,144);
				ident 			=   input_data.range(143,128);

				ts_tx.ts_sec 	=	input_data.range(127,96);
				ts_tx.ts_nsec	=	input_data.range(95,64);

				ts_rx.ts_sec 	=	input_data.range(63,32);
				ts_rx.ts_nsec	=	input_data.range(31,0);

				num_pack = num_pack +1;
				*num_packets = num_pack;


				L= L +COMPLETE_LENGTH;

				cout << "Ident "<< ident << "\tL=" << L <<"\tTS rx " << ts_rx.ts_sec << ts_rx.ts_nsec << "\tTS tx " << ts_tx.ts_sec << ts_tx.ts_nsec << endl;


				*length = L;

				
				payload_mul = (L) * CONST_1E9;

				sec_dif = ts_rx.ts_sec - ts_tx.ts_sec;
				
				delay = (ts_rx.ts_sec * CONT_CONV_NSEC + ts_rx.ts_nsec) - (ts_tx.ts_sec * CONT_CONV_NSEC + ts_tx.ts_nsec);


				sum_delay+=delay;

				*one_way_delay = sum_delay/num_pack;

				cout << "# " << ident;

				cout << "\tD " << *one_way_delay;


				if (num_pack > 1){

					if ((ident_1+1)==ident){
						time_stamps_diff_1 = time_stamps_diff;
						time_stamps_diff = (ts_rx.ts_sec * CONT_CONV_NSEC + ts_rx.ts_nsec)-(ts_rx_1.ts_sec * CONT_CONV_NSEC + ts_rx_1.ts_nsec);
						
						throughput_instant =  payload_mul / (time_stamps_diff);
						sum_thr += throughput_instant;
						num_pkt_thr += 1 ;
						*result = sum_thr/num_pkt_thr;

						cout << "\tthr: " << *result;
						*num_packets_throughput = num_pkt_thr;
					}

				}

				if (num_pack > 2){

					if (((ident_2+1)==ident_1) && ((ident_1+1)==ident)){

						jitter_instant = time_stamps_diff-time_stamps_diff_1;

						if (jitter_instant<0){
							jitter_instant=~jitter_instant+1; // absolute value

						}

						sum_jitter += jitter_instant;
						num_pkt_jitter += 1;
						*jitter = sum_jitter/num_pkt_jitter;
						*num_packets_jitter = num_pkt_jitter;

						cout << "\tIns J " << jitter_instant;
						cout << "\tJ: " << dec << *jitter;

						cout << "\tIns correct J " << jitter_instant;

					}


				}

				cout << endl << endl;

				
			}
		}
	} 

	cout << "end of stream:"<< dec << end_of_stream<< endl;
	return 0;
}
