#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include "myioctl.h"

/*
 * FunctionDescription: main function to write data to the character device using the write system call.
 * Function Arguments:
 *   None.
 * Function Return values:
 *   0 on success, negative error code on failure.
 */
int main()
{
	int fd,ret;
	int k=23;
	char buf[] = "CDAC";	//buffer to store the write data
	fd = open("/dev/test",O_RDWR);	//open the device file in read-write mode
	if(fd<0)
	{
		perror("device not found\n");	//error handling
		return fd;
	}

	ioctl(fd,CMD_1);	//invoke ioctl with CMD_1 command
	ret = write(fd,buf,sizeof(buf)); 	//write to the device file
	printf("\n wrote %d bytes",ret);	//print the number of bytes written

	return 0;
}
