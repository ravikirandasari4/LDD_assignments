#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

/*
 * FunctionDescription: main function to write data to the character device using the write system call.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
int main()
{
	int fd,ret;
	char buf[] = "Cool";			//buffer to store the write data
	fd = open("/dev/test",O_RDWR);	//open the device file in read-write mode
	if(fd<0)
	{
	 perror("device not found\n");
	 return fd;
	}
	ret = write(fd,buf,sizeof(buf));	//write to the device file
	printf("\n wrote %d bytes",ret);
	return 0;
}
