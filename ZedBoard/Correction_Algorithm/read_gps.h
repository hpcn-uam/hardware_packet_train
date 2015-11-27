#include <stdio.h>
#include <string.h>

#define epoch_year 1970

unsigned read_gps(void);
int parse_GPGGA(char buffer[]);
int parse_GPRMC(char buffer[]);
