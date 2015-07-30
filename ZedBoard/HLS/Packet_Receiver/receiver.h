/*******************************************************************************
 *
 *
 *  File:
 *        receiver.h
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
 
#ifndef _RECEIVE_H_
#define _RECEIVE_H_

#include <ap_int.h>
#include <hls_stream.h>
#include <stdint.h>

#include <iostream>

using namespace std;

using namespace hls;


#define ETH_INTERFACE_WIDTH 32

template<int D>
struct my_axis {
	ap_uint<D> data;
	ap_uint<D / 8> keep;
	ap_uint<1> last;
};

typedef my_axis<ETH_INTERFACE_WIDTH> axi_interface_type;

void packet_receiver
(
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

);
#endif
