#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include "packet.h"

using namespace std;

#define LENGTH_TEST 4500

int main (void){

	axi_interface_type *OUT_DATA;

	OUT_DATA = (axi_interface_type*) malloc (LENGTH_TEST * sizeof(axi_interface_type));


	ap_uint<48> mac_destination;
	ap_uint<48> mac_source;
	ap_uint<16> data_length;            // size of packet
	ap_uint<16> identification;
	ap_uint<3>	ip_flags;
	ap_uint<8>	time_to_live;
	ap_uint<32> ip_destination;
	ap_uint<32> ip_source;
	ap_uint<16> source_port;
	ap_uint<16> destination_port;
	ap_uint<32> Stimestamp;
	ap_uint<32> nStimestamp;
	ap_uint<32> num_packet;
	ap_uint<32>	packet_send;
	ap_uint<32> time_between_packet;
	ap_uint<1>  indefinitely;

	stream<axi_interface_type> data_out;

	int number_word,number_bytes;


	mac_destination= 0x1078D2EB2BFB;
	mac_source=0x001E4AE05200;
	ip_destination=0x96f43974;
	ip_source=0x272d7c39;
	source_port=0x0400;
	destination_port=0x220B;
	identification=25429;


	indefinitely=0;
	data_length=60;
	num_packet= 1;
	ip_flags=0;
	time_to_live=0x6D;


	Stimestamp=0xA85056DE;
	nStimestamp=0x987FED21;
	time_between_packet=0;

	packet_generator (
			data_out,
			&mac_destination,
			&mac_source,
			&data_length,            // size of packet
			&identification,
			&ip_flags,
			&time_to_live,
			&ip_destination,
			&ip_source,
			&source_port,
			&destination_port,
			&Stimestamp,
			&nStimestamp,
			&num_packet,
			&packet_send,
			&time_between_packet,
			&indefinitely);


	number_word = (data_length*8) / ETH_INTERFACE_WIDTH;
	number_bytes = ((data_length*8) % ETH_INTERFACE_WIDTH) /8;

	if (number_bytes!=0)
		number_word++;

	for (int o=0; o<num_packet; o++){
		printf("Packet %d of %d\n",o+1,(int)num_packet);

		for (int g=0; g< number_word; g++){
			data_out.read_nb(OUT_DATA[g]);

			cout << "Word: " << dec << g;
			cout << " = "<< hex <<  OUT_DATA[g].data ;
			cout << "\tkeep: " << OUT_DATA[g].keep;
			cout << "\tlast: " <<  OUT_DATA[g].last << endl;


		}

	}

	return 0;

}
