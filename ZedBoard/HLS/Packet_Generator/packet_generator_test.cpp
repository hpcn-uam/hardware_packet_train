/*******************************************************************************
 *
 *
 *  File:
 *        packet_generator_test.cpp
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
 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "packet.h"

using namespace std;

#define LENGTH_TEST 4500

#define fixed_paylod 18
#define udp_length    8
#define ip_header	 20
#define fixed_lenght 42

int main (void){

	axi_interface_type *OUT_DATA,*OUT_CONTROL;
	OUT_DATA= (axi_interface_type*) malloc(LENGTH_TEST * sizeof(axi_interface_type));
	OUT_CONTROL= (axi_interface_type*) malloc(LENGTH_TEST * sizeof(axi_interface_type));

	int32_t	pattern[10];
	int j,i,o,error=0,byte;
	char value[16];


	ap_uint<32> source_mac1,destination_mac1,timestamp;
	ap_uint<32> source_mac2,destination_mac2;
	ap_uint<32> aux;
	ap_uint<16> data_length,source_port,destination_port;
	ap_uint<16> identification;
	ap_uint<32> paquet_send,num_packet;
	ap_uint<32> ip_destination,ip_source;
	ap_uint<8> time_to_live,diff_serv;
	ap_uint<32> stimestamp,nstimestamp,time_between_packet;
	ap_uint<8> ip_flags;
	ap_uint<1> indef=0;

	stream<axi_interface_type> data_out, control_out;
	ap_uint<32> aux_show;

	int length_ip,length_paylod;

	FILE *pt;
	pt=fopen("udp.txt","r");

	if (!pt){
		printf ("Can't read file\n");
		exit (-1);
	}

	for (j=0;j<12;j++){
		fscanf(pt,"%s",value);
		switch(j){
			case 0:
				destination_mac1=strtoul(value,NULL,16);
				fscanf(pt,"%s",value);
				//printf("Destination MAC2 0x%.8X\n",(int)destination_mac2);
				break;
			case 1:
				source_mac1 =strtoul(value,NULL,16);
				fscanf(pt,"%s",value);
				source_mac2 =strtoul(value,NULL,16);
				break;
			case 2:
				diff_serv=strtoul(value,NULL,16);
				break;
			case 3:
				data_length=strtoul(value,NULL,10);
				printf("data length %d\n",(int)data_length);
				break;
			case 4:
				identification=strtoul(value,NULL,16);
				break;
			case 5:
				ip_flags=strtoul(value,NULL,16);
				//printf("Ip flags %d\n",(uint)ip_flags);
				break;
			case 6:
				time_to_live=strtoul(value,NULL,16);
				break;
			case 7:
				ip_source=strtoul(value,NULL,16);
				break;
			case 8:
				ip_destination=strtoul(value,NULL,16);
				break;
			case 9:
				source_port=strtoul(value,NULL,16);
				break;
			case 10:
				destination_port=strtoul(value,NULL,16);
				break;
			case 11:
				num_packet=strtoul(value,NULL,16);
				break;

		}

	}


	stimestamp =0xABC56DE;
	nstimestamp =0x987FED21;
	time_between_packet=0;


	for (j=0;j<10;j++){
		fscanf(pt,"%s",value);
		pattern[j]=strtoul(value,NULL,16);
	}

	byte=(data_length/4);

	if ((data_length%4)!=0){
		byte++;
	}

	cout << "Start Test" << endl;

	packet_generator (data_out,control_out,&destination_mac1,
			&destination_mac2,&source_mac1,&source_mac2,
			&diff_serv,&data_length,&identification,&ip_flags,
			&time_to_live,&ip_destination,&ip_source,&source_port,
			&destination_port,&stimestamp,&nstimestamp,&num_packet,
			&paquet_send,&time_between_packet,&indef);


	for(o=0;o<num_packet;o++){
		printf("Packet %d de %d\n",o+1,(int)num_packet);

		for (i=0;i<6;i++){
			control_out.read(OUT_CONTROL[i]);
			printf("Control Word:%d = 0x%.8X keep: 0x%.1X last:%d\n",i,(int)OUT_CONTROL[i].data,
					(int)OUT_CONTROL[i].keep,(int)OUT_CONTROL[i].last);
		}

		for (i=0;i<(((fixed_lenght+fixed_paylod)/4)+byte);i++){
			data_out.read_nb(OUT_DATA[i]);
			aux_show=OUT_DATA[i].data;
//			printf("Word:%2d = 0x%.8X keep: 0x%.1X  last:%d\n",i,(int)OUT_DATA[i].data,
//						(int)OUT_DATA[i].keep,(int)OUT_DATA[i].last);

			printf("Word:%2d = 0x%.8X keep: 0x%.1X  last:%d\n",i,
					(int)(aux_show.range(7,0),aux_show.range(15,8),
						  aux_show.range(23,16),aux_show.range(31,24)),
					(int)OUT_DATA[i].keep,(int)OUT_DATA[i].last);



			if (i==4){
//				length_ip= ((int)(OUT_DATA[i].data & 0xFF00)>>8)+(OUT_DATA[i].data&0xFF);
				length_ip=(int)(aux_show.range(7,0),aux_show.range(15,8));
			}

			if (i==9){
//				length_paylod = ((int)OUT_DATA[i].data >>24) +((( int)OUT_DATA[i].data &0xFF0000 )>>16);
				length_paylod = (int)(aux_show.range(23,16),aux_show.range(31,24));
			}


//			if (((int)OUT_DATA[i].data!=pattern[i])&&(i<10)){
//				printf("Error Word %d\n",i);
//				error++;
//			}

		}
		printf("Length IP header %d\n",length_ip);
		printf("Length UDP header %d\n",length_paylod);
	}
	printf("Packets send: %u\n",(uint32_t)paquet_send);
	fclose(pt);
	return (0);


}


