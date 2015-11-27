#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <getopt.h>
#include <math.h>

#include "read_gps.h"

#define UIO_SIZE "/sys/class/uio/uio0/maps/map0/size"
#define uio	"/dev/uio0"


#define MAX_NANOSEC	1000000000

#define Sec				0x0				// only read
#define NanoSec			0x1				// only read
#define Correc			0x2				// write
#define Second2load		0x3				// write
#define Err				0x4				// write
#define	Fixed			0x5				// write (only bit 0)
#define	Signed_Error	0x6				// write (only bit 0)
#define Corr_En			0x7				// write (only bit 0)
#define Mode			0x8				// write (only bit 0)


float division;

unsigned sec2load;





char text[50];
int jota=0,te=0;
FILE *ptr;

int main(int argc , char **argv) {
	
	
	int interrupt,reenable;
	int uio_fd,fixed=0;
	int error,error_1=0,error_2=0;
	unsigned int uio_size;
	FILE *size_fp;
	void *base_address;
	
	int opt, show=0,save=0;

	while ((opt= getopt (argc,argv , "hsw")) !=EOF){
		switch (opt){
			case 'h':
				printf ("Help Program Receive Packet's\n"
						"Valid Arguments are:\n"
						"-s flag\t: show on terminal Second, Nanosecond, Error, Correction and Action of Control\n"
						"-w flag\t: write in file Second, Nanosecond, Error, Correction and Action of Control\n");
				return 0;
			case 's':
				show = 1;
				break;
			case 'w':
				save=1;
				break;	
			}
	}
	

	int nanosecond,second;
	float correction,u_k=0.0;	
	float Avg_Error,Avg_Error_prev=0.0,Avg_Mean_Pend=0.0;


	sprintf(text,"/lost+found/clock%d",te);

	if ((ptr=fopen(text,"w"))==NULL){
		printf("Can not create File\n");
		return -1;
	}

	uio_fd= open(uio,O_RDWR);
	size_fp=fopen(UIO_SIZE, "rw");
	fscanf(size_fp, "0x%08X", &uio_size);
	
	base_address=mmap(NULL,uio_size,PROT_READ | PROT_WRITE,MAP_SHARED,uio_fd, 0);

	((int*)base_address)[Mode]=0;					// Start counter
	((int*)base_address)[Corr_En]=0;				// Enable correction
	
	//	read gps

	if ((sec2load=read_gps())==-1) 
		return -1;
		
	printf("Secont to load %u\n\n",sec2load);
	

	
	((int*)base_address)[Second2load]=sec2load;				// Enable correction
		
	usleep(50);	
	((int*)base_address)[Mode]=1;					// Start counter
	((int*)base_address)[Corr_En]=1;				// Enable correction

	

	system ("echo 7 > /sys/class/gpio/export");
	system ("echo out > /sys/class/gpio/gpio7/direction");
	
	if (show==1)
		printf("Second\t\t Nanosecond\t\t Error\t\t Correction\t\t Action of Control\n");
	if (save==1)
		fprintf(ptr,"Second\t\t Nanosecond\t\t Error\t\t Correction\t\t Action of Control\n");	

	while (1){


		write(uio_fd,&reenable,sizeof(int));
	
		second = ((int*)base_address)[Sec];

		nanosecond = ((int*)base_address)[NanoSec];
		


		if (nanosecond >  MAX_NANOSEC/2)
			error = ~(MAX_NANOSEC - nanosecond)+1;
		else
			error = nanosecond;	

/* Start Algoritmh */


		Avg_Error_prev=Avg_Error;						//mean with 15 previous values

		if (error > 0){
			Avg_Error=ceil(0.94*Avg_Error_prev+0.06*(correction+error-error_1));
		}
		else{
			Avg_Error=floor(0.94*u_k+0.06*(correction+error-error_1));
		}
		
		Avg_Mean_Pend=round(0.7*(Avg_Error-Avg_Error_prev)+Avg_Mean_Pend*0.3);	//slope of mean

		correction=(Avg_Error+round(0.26*error)+Avg_Mean_Pend);
		
		

		division = ((-1) * (MAX_NANOSEC/(correction)));	 


/* End Algoritmh */

	
		
		((int*)base_address)[Correc]=(int)division;

		if (show==1)
			printf("%12d\t %12d\t %12d\t %12d\t\t\t %12d\n",second,nanosecond,error,(int)correction,(int)division);
		if (save==1)
			fprintf(ptr,"%12d\t %12d\t %12d\t %12d\t\t\t %12d\n",second,nanosecond,error,(int)correction,(int)division);
		
		read(uio_fd,&interrupt,sizeof(int));   							// interrupt
		
		if (abs(error)<50)
			fixed++;
		else
			fixed=0;	

		if (fixed >10){
			system ("echo 1 > /sys/class/gpio/gpio7/value");		// turn on green led! fixed!!!
		}
		else
			system ("echo 0 > /sys/class/gpio/gpio7/value");

		error_2= error_1;
		error_1 = error;
		u_k = correction;
		
		if (jota++==3600){
			jota=0;
			te++;
			fclose(ptr);
			
			sprintf(text,"/lost+found/clock%d",te);

			if ((ptr=fopen(text,"w"))==NULL){
				printf("Can not create File\n");
				return -1;
			}
			
			fprintf(ptr,"Second\t\t Nanosecond\t\t Error\t\t Correction\t\t Action of Control\n");
		}
			
	}
		

	munmap(base_address,uio_size);
	fclose (size_fp);
	fclose(ptr);
}
