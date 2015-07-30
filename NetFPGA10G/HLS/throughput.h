/*******************************************************************************
 *
 *
 *  File:
 *        throughput.h
 *
 *
 *  Module:
 *        Network Parameters Calculator
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

#include <ap_int.h>
#include <hls_stream.h>
#include <stdint.h>

using namespace std;
using namespace hls;

#define ETH_INTERFACE_WIDTH 160
#define CONST_1E9 1000000000;
#define CONT_CONV_NSEC 4294967296
#define COMPLETE_LENGTH 14;

template<int D>
struct my_axis {
	ap_uint<D> data;
	ap_uint<D/8> keep;
	ap_uint<1> last;
};

typedef my_axis<ETH_INTERFACE_WIDTH> axi_interface_type;



struct timestamping {
	ap_uint<32> ts_sec;
	ap_uint<32> ts_nsec;
};


int throughput_calc (
		stream <axi_interface_type> &in_data,
		ap_uint<16> 				*num_packets,
		ap_uint<16>					*num_packets_jitter,
		ap_uint<16>					*num_packets_throughput,
		ap_uint<16>					*one_way_delay,
		ap_uint<16>					*jitter,
		ap_uint<16>					*length,
		ap_uint<34> 				*result,
		ap_uint<1>  				*end_calculations );
