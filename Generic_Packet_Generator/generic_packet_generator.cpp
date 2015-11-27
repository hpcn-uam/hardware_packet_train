#include "packet.h"

using namespace std;
using namespace hls;


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

){

#pragma HLS RESOURCE variable=out_data core=AXIS metadata="-bus_bundle M_AXIS_RXD"

#ifdef CONTROL_INTERFACE
#pragma HLS RESOURCE variable=out_control core=AXIS metadata="-bus_bundle M_AXIS_RXS"
#endif


#pragma HLS RESOURCE variable=mac_destination core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
#pragma HLS RESOURCE variable=mac_source core=AXI4LiteS metadata="-bus_bundle S_AXI_LITE"
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


	axi_interface_type aux;

	ap_uint<512> base_packet=0;

	ap_uint<16> total_length;
	ap_uint<16> data_length_aux;
	ap_uint<16> packet_identificator;
	ap_uint<16> total_length_upd;
	ap_uint<ETH_INTERFACE_WIDTH/8> aux_keep=0;

	ap_uint<32> aux_check_sum;
	ap_uint<16> chk_sum;

	int number_word; // word to send
	int number_bytes;
	int number_word_header;

	ap_uint<176> filled=0;

	int borrar;


	/* Parameters asignation */

	packet_identificator = *identification;

	if (*data_length<60)
			data_length_aux=60;
		else
			data_length_aux= *data_length;

	total_length= data_length_aux-MAC_HEADER; // length of IP header
	total_length_upd=data_length_aux-MAC_HEADER-IP_HEADER;



	base_packet.range(511,464)=*mac_destination;
	base_packet.range(463,416)=*mac_source;
	base_packet.range(415,400)=TYPE;
	base_packet.range(399,396)=VERSION;
	base_packet.range(395,392)=HDR_LEN;
	base_packet.range(391,384)=TYPE_SERVICE;
	base_packet.range(383,368)=total_length;
	base_packet.range(351,349)=*ip_flags;
	base_packet.range(348,336)=FRAGMENT_OFFSET;
	base_packet.range(335,328)=*time_to_live;
	base_packet.range(327,320)=PROTOCOL;
	base_packet.range(303,272)=*ip_source;
	base_packet.range(271,240)=*ip_destination;
	base_packet.range(239,224)=*source_port;
	base_packet.range(223,208)=*destination_port;
	base_packet.range(207,192)=total_length_upd;
	base_packet.range(367,352)=packet_identificator;


	number_word = (*data_length*8) / ETH_INTERFACE_WIDTH;
	number_bytes = ((*data_length*8) % ETH_INTERFACE_WIDTH) /8;
	number_word_header = WIDTH_BASE/ETH_INTERFACE_WIDTH;
	if (number_bytes!=0)
		number_word++;



	aux.keep=0;
	aux_check_sum=0;

calc_keep: for (int s=0 ; s < (ETH_INTERFACE_WIDTH/8); s++){

	#pragma HLS UNROLL
			aux.keep += 1 << s;
			if (s<number_bytes)
				aux_keep += 1 << s;
		}

chk: for(int i=240; i<399 ;i+=16){
		#pragma HLS UNROLL
			aux_check_sum += base_packet.range(i+15,i);
		}
		chk_sum = ~(aux_check_sum.range(31,16)+aux_check_sum.range(15,0));


number_pkt: for (int l=0; l<(uint32_t)*num_packet; l++ ){
#pragma HLS LOOP_TRIPCOUNT min=1 max=10

	base_packet.range(367,352)=packet_identificator+l;
	aux.last=0x0;

	gen_pkt:for (int j=0; j < number_word ; j++){
#pragma HLS PIPELINE

				if (j==0){
					/* Timestamp */
					base_packet.range(175,144)=*Stimestamp;
					base_packet.range(143,112)=*nStimestamp;
					base_packet.range(319,304)= chk_sum-l;
					base_packet.range(367,352)=packet_identificator+l;
					cout << "Check sum "<< hex << base_packet.range(319,304) << endl;
				}



				if (j<number_word_header){

					assign: for (int h=0; h< ETH_INTERFACE_WIDTH; h+=8){
#pragma HLS UNROLL
						aux.data.range(h+7,h)=base_packet.range(511-h-j*ETH_INTERFACE_WIDTH,511-h-j*ETH_INTERFACE_WIDTH-7);
					}

				}
				else{
					aux.data=ETH_INTERFACE_WIDTH*j;
				}

				if (number_word-1==j){
					aux.last=1;
					if (number_bytes!=0)
						aux.keep=aux_keep;
				}

				out_data.write(aux);

		}
		*packet_send=l;

		delay: for (int h=0; h< *time_between_packet; h++){
#pragma HLS INLINE
			borrar++;
		}
	}


}
