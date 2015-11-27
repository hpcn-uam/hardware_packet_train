#include "read_gps.h"




unsigned read_gps(void){

	FILE *ptr;
	char uart[100];
	char gpgga[100];
	char gprmc[100];
	int i=0;
	int gp_gga=0,gp_rmc=0;
	int a;



	ptr=fopen ("/dev/ttyPS1","r");
	if (ptr==NULL){
		printf("Can not Read GPS\n");
	}

	while ((uart[0]=fgetc(ptr))!='$');


	for (i=1;;i++){
		uart[i]=fgetc(ptr);
		if (i==5){

			if (((strstr(uart,"$GPGGA"))!=NULL) && (gp_gga==0)){

				gp_gga=1;
			}
			else if (((strstr(uart,"$GPRMC"))!=NULL && (gp_rmc==0))){

				gp_rmc=1;
			}
			else {
				while ((uart[0]=fgetc(ptr))!='$');
				i=0;
			}
		}
		else if(i>6){

			if (uart[i]=='$'){

				if ((strstr(uart,"$GPGGA"))!=NULL && gp_gga==1){

					strcpy(gpgga,uart);
					uart[0]='$';
					i=0;
					if (gp_rmc==1){
						break;
					}
				}
				else if (((strstr(uart,"$GPRMC"))!=NULL) && gp_rmc==1){
					strcpy(gprmc,uart);
					uart[0]='$';
					i=0;
					if (gp_gga==1){
						break;
					}
				}
			}
		}
	}


	if ((a= parse_GPGGA(gpgga))==-1){
		printf("No signal of GPS, please wait a moment and try again\n");
		return -1;
	}

	a= parse_GPRMC(gprmc);

	printf("Actual second \t\t| %u\n",a);


	return a;
}



int parse_GPGGA(char buffer[]){

	char *ptr;
	char *ptr1;
	int i=0;
	int o=0;
	int aux;
	char lat[30],longi[30];
	int sat_used;

	if (strstr(buffer,",,,")!=NULL)
		return -1;

	ptr=strtok(buffer,",");

	while((ptr=strtok(NULL,",")) !=NULL) {
		switch (i){
			case 1:
				strcpy(lat,ptr);
				break;
			case 2:
				ptr1=ptr;
				break;
			case 3:
				strcpy(longi,ptr);
				break;
			case 4:
				if ((strstr(ptr1,"N"))!=NULL)
					printf("Position \t\t| ");
				else
					printf("Position \t\t| -");
				while (lat[o]!='\0'){
					printf("%c",lat[o]);
					if (o==1)
						printf(" ");
					o++;
				}
				if ((strstr(ptr,"E"))!=NULL)
					printf(" , ");
				else
					printf(" , -");
				o=0;
				while (longi[o]!='\0'){
					printf("%c",longi[o]);
					if (o==2)
						printf(" ");
					o++;
				}
				printf("\n");
				break;
			case 5:
				sscanf(ptr,"%d",&aux);
				if (aux==0){
					printf("No valid Data\n");
				}
				else if (aux==1){
					printf("GPS Position FIX\n");
				}
				else if (aux==2){
					printf("DGPS Position FIX\n");
				}
				break;
			case 6:
				sscanf(ptr,"%d",&sat_used);
				printf("Satellites used \t| %d\n",sat_used);
				if(sat_used<4){
					printf("Warning! There is no minimum number of satellites\n");
				}
				break;
			case 7:
				printf("HDOP \t\t\t| %s\n",ptr);
				break;
			case 8:
				printf("MSL Altitude \t\t| %s",ptr);
				break;
			case 9:
				printf(" %s\n",ptr);
				break;
			case 10:
				printf("Geoidal Separation \t| %s",ptr);
				break;
			case 11:
				printf(" %s\n",ptr);
				return 0;
		}
		i++;
	}
	return 0;
}

int parse_GPRMC(char buffer[]){
	char *ptr;
	int aux,i=0;
	int day,month,year;
	int hour,min,sec;
	int actual_day=0;
	unsigned actual_second;
	

	ptr=strtok(buffer,",");

	while((ptr=strtok(NULL,",")) !=NULL) {
		switch (i){
			case 0: // time
				sscanf(ptr,"%d",&aux);
				hour=(int)(aux/10000);
				min=(int)((aux-hour*10000)/100);
				sec=(int)(aux-hour*10000-min*100);
				hour+=1;		// GTM+1
				printf("GMT +1 \t\t\t| %.2u:%.2u:%.2u\n",hour,min,sec);
				break;
			case 8:
				sscanf(ptr,"%d",&aux);
				day= (int)(aux/10000);
				month=(int)((aux-day*10000)/100);
				year=(int)(aux-day*10000-month*100)+2000;
				printf("Date: \t\t\t| %.2u/%.2u/%.4u\n",day,month,year);
		}
		i++;
	}

	actual_day= ((year-epoch_year)*365)+(int)((year-epoch_year)/4);

	switch(month){
		case 1:
			actual_day+=day; // january
			break;
		case 2:
			actual_day+=day+31; // february
			break;
		case 3:
			actual_day+=day+59; //march
			break;
		case 4:
			actual_day+=day+90; //april
			break;
		case 5:
			actual_day+=day+120; //may
			break;
		case 6:
			actual_day+=day+151; //june
			break;
		case 7:
			actual_day+=day+181; //july
			break;
		case 8:
			actual_day+=day+212; //august
			break;
		case 9:
			actual_day+=day+243; //september
			break;
		case 10:
			actual_day+=day+273; //october
			break;
		case 11:
			actual_day+=day+304; //november
			break;
		case 12:
			actual_day+=day+334; //december
			break;
	}


	actual_second=actual_day*86400+hour*3600+min*60+sec;

	return actual_second;

}




