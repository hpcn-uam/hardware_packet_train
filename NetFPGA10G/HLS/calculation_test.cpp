/*******************************************************************************
 *
 *
 *  File:
 *        calculation_test.cpp
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

#include "throughput_calc.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>


#define LENGTH_TEST 500
#define ONE_SEC 1000000000

int main()
{

	axi_interface_type *IN_DATA;
	IN_DATA= (axi_interface_type*) malloc((LENGTH_TEST) * sizeof(axi_interface_type));

	stream<axi_interface_type> data_in;
	ap_uint<34>  result;
	ap_uint<1> end_calculation=0;

	ap_uint<32> tx_sec=2, rx_sec=2,tx_nsec=103638574,rx_nsec=103639803;

	ap_uint<16> L,ident,length;


	ap_uint<16> num_pkts,num_pkts_jitter,num_pkts_throughput,one_way_delay,jitter;

	ap_uint<160> test;

	FILE *tmp;

	//test = (L,tx_sec,tx_nsec,rx_sec,rx_nsec);

    long long Long_i, Long_ii, dif_1sec;
    int i,j, dif,data=0;


    if ((tmp=fopen("data.txt","r"))==NULL) {
    	cout << endl << endl << "Error opened File"<< endl << endl;
    	return -1;
    }

    cout << "Correct open file" << endl;

    while (!feof(tmp)) {
    	fscanf(tmp, "%X\n", &ident);
    	fscanf(tmp, "%X\n", &L);
        fscanf(tmp, "%X\n", &tx_sec);
        fscanf(tmp, "%X\n", &tx_nsec);
        fscanf(tmp, "%X\n", &rx_sec);
		fscanf(tmp, "%X\n", &rx_nsec);



		IN_DATA[data].data =(ident,L,tx_sec,tx_nsec,rx_sec,rx_nsec);

		IN_DATA[data].keep=0xFFFFF;
		IN_DATA[data++].last=0;
    }

    cout << "Read all data" << endl;

	for(i=0;i<data-1;i++){
		data_in.write(IN_DATA[i]);
		/*cout <<"pkt:"<<dec<<i << "\t data:" << hex <<IN_DATA[i].data << "\t keep:"<<hex<<IN_DATA[i].keep <<
				"\tlast:" << dec<< IN_DATA[i].last << endl;*/

	}


	throughput_calc (data_in,
					&num_pkts,
					&num_pkts_jitter,
					&num_pkts_throughput,
					&one_way_delay,
					&jitter,
					&length,
					&result,
					&end_calculation);




	cout << "Number of Packets is " << dec << num_pkts << endl;
	cout << "The Final Result is: " << dec << result*8 << " Bits per sec \n"  << endl;




	return 0;

}

