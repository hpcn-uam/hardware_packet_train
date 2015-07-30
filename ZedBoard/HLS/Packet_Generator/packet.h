/*******************************************************************************
 *
 *
 *  File:
 *        packet.h
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
 
#ifndef _PACKET_H_
#define _PACKET_H_

using namespace std;

#include <ap_int.h>
#include <hls_stream.h>
#include <stdint.h>

using namespace hls;
#define ETH_INTERFACE_WIDTH 32

template<int D>
struct my_axis {
	ap_uint<D> data;
	ap_uint<D / 8> keep;
	ap_uint<1> last;
};

typedef my_axis<ETH_INTERFACE_WIDTH> axi_interface_type;

void packet_generator
(
		stream <axi_interface_type> &out_data,
		stream <axi_interface_type>	&out_control,
		ap_uint<32> *mac_destination1,
		ap_uint<32> *mac_destination2,
		ap_uint<32> *mac_source1,
		ap_uint<32> *mac_source2,
		ap_uint<8>	*diff_serv,
		ap_uint<16> *data_length,
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
		ap_uint<1> * indefinitely
);
#endif
