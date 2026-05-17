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
	char buf[SIZE];
	fd = open("/dev/test",O_RDWR|O_NONBLOCK);	//open the device file in read-write and non-blocking mode
	if(fd<0)
	{
		perror("device not found\n");
		return fd;
	}
	ret = read(fd,buf,SIZE);	//read from the device file and store the data in the buffer, ret will contain the number of bytes read or a negative error code
	printf("\n READ FROM KERNEL %s  count %d\n",buf,ret);
	return 0;
}
