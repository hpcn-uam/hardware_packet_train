#ifndef _PACKET_H_
#define _PACKET_H_

#include <ap_int.h>
#include <hls_stream.h>
#include <stdint.h>

using namespace std;
using namespace hls;

#define ETH_INTERFACE_WIDTH 32


template<int D>
struct my_axis {
	ap_uint<D> data;
	ap_uint<D/8> keep;
	ap_uint<1> last;
};

typedef my_axis<ETH_INTERFACE_WIDTH> axi_interface_type;


const ap_uint<4>  VERSION=4;
const ap_uint<4>  HDR_LEN=5;
const ap_uint<13> FRAGMENT_OFFSET=0;
const ap_uint<8>  PROTOCOL=0x11;
const ap_uint<16> TYPE=0x0800;
const ap_uint<8>  TYPE_SERVICE=0;
const ap_uint<16> UDP_LENGTH=8;

#define FIXED_PAYLOAD 18
#define IP_HEADER	  20
#define FIXED_LENGTH  42
#define MAC_HEADER    14

#define WIDTH_BASE 480

void packet_generator(
		stream <axi_interface_type> &out_data,
#ifdef CONTROL_INTERFACE
		stream <axi_interface_type>	&out_control,
#endif
		ap_uint<48> *mac_destination,
		ap_uint<48> *mac_source,
		ap_uint<16> *data_length,            // size of packet
		ap_uint<16> *identification,
		ap_uint<3>	*ip_flags,
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

);

#endif
