// Shared .so library used by python Raspi4_DALI_Controller_int
//functions: C_DALI_write() & C_DALI_read()
#include <stdio.h>
#include <fcntl.h>
 
//gcc -fPIC -shared -o libdali.so DALI_C_functions.c

int fd;


int C_DALI_write(int DALI_command)
{
	char buffer[10];

	printf("DALI_command is %d\n",DALI_command);

	fd = open("/dev/my_DALI_drv", O_RDWR );

	if( fd < 0 ){
    printf("Cannot open device \t"); printf(" fd = %d \n",fd); return -1;}

	sprintf(buffer,"%d",DALI_command);
	write( fd, buffer, 1 );
  
	return 0;
}


int C_DALI_read()
{
	char gpio_buffer[10];
	read( fd, gpio_buffer, 1);
	return atoi(gpio_buffer);
}
