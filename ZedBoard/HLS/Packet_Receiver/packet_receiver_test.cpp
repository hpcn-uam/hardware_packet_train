/*******************************************************************************
 *
 *
 *  File:
 *        packet_receiver_test.cpp
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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define LENGTH_TEST 4500

int main(void){
	axi_interface_type *IN_DATA,*IN_CONTROL,*OUT_DATA,*OUT_CONTROL;
	OUT_DATA= (axi_interface_type*) malloc(LENGTH_TEST * sizeof(axi_interface_type));
	OUT_CONTROL= (axi_interface_type*) malloc(LENGTH_TEST * sizeof(axi_interface_type));
	IN_DATA= (axi_interface_type*) malloc(LENGTH_TEST * sizeof(axi_interface_type));
	IN_CONTROL= (axi_interface_type*) malloc(LENGTH_TEST * sizeof(axi_interface_type));

	stream<axi_interface_type> crtl_in,data_in,crtl_out,data_out;

	FILE *pt_ctrl,*pt_dt;
	char text_read[10];
	int i,j,error,b;
	int z,g;

		ap_uint<32> mac_destination1=0x000FFECA;//0x000FFECA
		ap_uint<32> mac_destination2=0xA5E5;
		ap_uint<32> mac_source1=0x001E;
		ap_uint<32> mac_source2=0x4AE05200;
		ap_uint<32> ip_source=0xBBFCDCC4;
		ap_uint<32> ip_destination=0x96F43841;
		ap_uint<16> source_port=0x1CFC;
		ap_uint<16> destination_port=0x5016;
		ap_uint<32> Stimestamp=0x13579bd;
		ap_uint<32> nStimestamp=0x02468ac;
		ap_uint<32> received_packets=0;
		ap_uint<32> untagged_packets;
		ap_uint<32> market_packets;
		ap_uint<1>	analisys_mac=1;
		ap_uint<1> reset_values=0;
		ap_uint<32> pos_read=0;
		ap_uint<32> element_value;


	for (b=0;b<3;b++){

		for (i=0;i<LENGTH_TEST;i++){
			IN_DATA[i].data=0;
			IN_CONTROL[i].data=0;
			OUT_DATA[i].data=0;
			OUT_CONTROL[i].data=0;

			IN_DATA[i].keep=0;
			IN_CONTROL[i].keep=0;
			OUT_DATA[i].keep=0;
			OUT_CONTROL[i].keep=0;

			IN_DATA[i].last=0;
			IN_CONTROL[i].last=0;
			OUT_DATA[i].last=0;
			OUT_CONTROL[i].last=0;

		}

	//	printf("Start Test\n");


		if ((pt_ctrl=fopen("control","r"))==NULL){
			printf("Can't read control file\n");
		}
		if ((pt_dt=fopen("packet","r"))==NULL){
			printf("Can't read packet file\n");
		}

		i=0;
		while (!feof(pt_ctrl)){
			fscanf(pt_ctrl,"%s",text_read);
			IN_CONTROL[i].data = strtoul(text_read,NULL,16);
			fscanf(pt_ctrl,"%s",text_read);
			IN_CONTROL[i].keep = strtoul(text_read,NULL,16);
			fscanf(pt_ctrl,"%s",text_read);
			IN_CONTROL[i].last = strtoul(text_read,NULL,2) & 0x1;
			crtl_in.write(IN_CONTROL[i]);
			i++;
		}


		j=0;
		while (!feof(pt_dt)){
			fscanf(pt_dt,"%s",text_read);
			if (b%2)
				IN_DATA[j].data = strtoul(text_read,NULL,16);
			else
				IN_DATA[j].data = strtoul(text_read,NULL,16)+1;
			fscanf(pt_dt,"%s",text_read);
			IN_DATA[j].keep = strtoul(text_read,NULL,16);
			fscanf(pt_dt,"%s",text_read);
			IN_DATA[j].last = strtoul(text_read,NULL,16) & 0x1;
			data_in.write(IN_DATA[j]);

			j++;
		}


		fclose (pt_ctrl);
		fclose (pt_dt);



		packet_receiver(
				data_in,
				crtl_in,
				data_out,
				crtl_out,
				&mac_destination1,
				&mac_destination2,
				&mac_source1,
				&mac_source2,
				&ip_destination,
				&ip_source,
				&source_port,
				&destination_port,
				&Stimestamp,
				&nStimestamp,
				&received_packets,
				&untagged_packets,
				&market_packets,
				&analisys_mac,
				&reset_values,
				&pos_read,
				&element_value);



			for (z=0;z<i;z++){
				crtl_out.read_nb(OUT_CONTROL[z]);
			}

			for (g=0;g<j;g++){
				data_out.read_nb(OUT_DATA[g]);
				printf("Data   : %2d 0x%.8X keep 0x%X last %d",g,(int)OUT_DATA[g].data,(int)OUT_DATA[g].keep,(int)OUT_DATA[g].last);
				if ((OUT_DATA[g].data!=IN_DATA[g].data)||(OUT_DATA[g].keep!=IN_DATA[g].keep)
						||(OUT_DATA[g].last!=IN_DATA[g].last)){
					if (g!=10 && g!=13 && g!=14) {
						error++;
						printf (" Error");
					}

				}


				if (g==10){
					printf (" Number of timestamp");
				}

				if (g==13){
					if (OUT_DATA[g].data!=(uint32_t)(Stimestamp.range(7,0),Stimestamp.range(15,8),
							Stimestamp.range(23,16),Stimestamp.range(31,24))) {
						printf (" Error bad timestamp");
					}
					else
						printf (" timestamp OK");
				}
				if (g==14){
					if (OUT_DATA[g].data!=(uint32_t)(nStimestamp.range(7,0),nStimestamp.range(15,8),
							nStimestamp.range(23,16),nStimestamp.range(31,24))) {
						printf (" Error bad timestamp");
					}
					else
						printf (" timestamp OK");
				}

				printf("\n");
			}

		cout << "iteration : " << dec << b << endl;

	}


	printf ("Packet Received: %d\n",(int)received_packets);
	printf ("Packet Untagged : %d\n",(int)untagged_packets);
	printf ("Packet Market : %d\n",(int)market_packets);


	return 0;
}
