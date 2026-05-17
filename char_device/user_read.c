#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

#define SIZE 8

/*
 * FunctionDescription: main function to read data from the character device using the read system call.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
int main()
{
	int fd,ret;
	char buf[SIZE];	//buffer to store the read data
	fd = open("/dev/test",O_RDWR);	//open the device file
	if(fd<0)
	{
	 perror("device not found\n");	//error handling
	 return fd;
	}
	ret=read(fd,buf,SIZE);	//read from the device file
	printf("\n READ FROM KERNEL %s  count %d\n",buf,ret);	//print the read data
	return 0;
}
