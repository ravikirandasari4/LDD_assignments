#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

#define SIZE 8

/*
 * FunctionDescription: producer main function to write data to the character device using the write system call.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
int main()
{
	int fd,ret;
	char buf[SIZE];								//buffer to store the read data
	fd = open("/dev/test",O_RDWR|O_NONBLOCK);	//open the device file in read-write and non-blocking mode
	if(fd<0)
	{
		perror("device not found\n");
		return fd;
	}
	ret = read(fd,buf,SIZE);	//read from the device file
	printf("\n READ FROM KERNEL %s  count %d\n",buf,ret);
	return 0;
}
