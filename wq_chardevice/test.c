#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/circ_buf.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<linux/wait.h>
#include<linux/sched.h>

#define SIZE 8

//global variables for device number, character device structure, circular buffer, and wait queue
wait_queue_head_t twq;
dev_t devno;
struct cdev _cdev;
struct circ_buf cbuf;

//function prototypes for file operations
int test_open (struct inode *, struct file *);
int test_close (struct inode *, struct file *);
ssize_t test_read (struct file *, char __user *, size_t, loff_t *);
ssize_t test_write (struct file *, const char __user *, size_t, loff_t *);

//file operations structure to link the system calls to the corresponding functions in the driver
struct file_operations fops={
	.open = test_open,		//pointer to the function to handle open system call
	.release = test_close,	//pointer to the function to handle close system call
	.read = test_read,		//pointer to the function to handle read system call
	.write = test_write		//pointer to the function to handle write system call
};

/*
 * FunctionDescription: Called when the device file is opened.
 * Function Arguments: 
 *  inodep - pointer to the inode representing the device.
 *  filep - pointer to the file structure for the opened device.
 * Function Return values:
 *  0 on success.
 */
int test_open(struct inode* inodep,struct file* filep)
{
	printk("\n in open function\n");
	printk("\n cdev in inode of i_cdev %p\n",inodep->i_cdev);
	printk("\n cdev address in drive %p\n",&_cdev);
	return 0;
}

/*
 * FunctionDescription: Called when the device file is closed.
 * Function Arguments: 
 *  inodep - pointer to the inode representing the device.
 *  filep - pointer to the file structure for the opened device.
 * Function Return values:
 *  0 on success.
 */
int test_close(struct inode* inodep,struct file *filep)
{
	printk("\n in close function\n");
	return 0;
}

/*
 * FunctionDescription: Called when the device file is read.
 * Function Arguments: 
 *  filep - pointer to the file structure for the opened device.
 *  buffer - pointer to the user buffer where data will be copied.
 *  cnt - number of bytes to read.
 *  off - pointer to the file offset.
 * Function Return values:
 *  Number of bytes read on success, negative error code on failure.
 */
ssize_t test_read(struct file *filep,char __user *buffer,size_t cnt,loff_t *off)
 {
	int ret,i,m;
	printk("\n in read function\n");
	if(filep->f_flags&O_NONBLOCK)
	{
		if(CIRC_CNT(cbuf.head,cbuf.tail,SIZE)==0)	//check if the circular buffer is empty when the file is opened in non-blocking mode
		{ 
	    	return -EAGAIN;
		}
		else
	  	{ 
   	    	goto next;
	  	}
	}
	//wait until there is data in the circular buffer to read if the file is opened in blocking mode and the buffer is empty
	wait_event_interruptible(twq,CIRC_CNT(cbuf.head,cbuf.tail,SIZE)>=1);
next:
	m=min(CIRC_CNT(cbuf.head,cbuf.tail,SIZE),(int)cnt);	//calculate the number of bytes to read based on the circular buffer state and requested count
	for(i=0;i<m;i++)
	{
		ret=copy_to_user(buffer+i,cbuf.buf+cbuf.tail,1);	//copy data from kernel space to user space
		if(ret)
		{ 
	   		printk("\n error copying to user\n");
			return -ENOMEM;
		}
		printk("\n copied to user %c\n",buffer[i]);
		cbuf.tail=(cbuf.tail+1) & (SIZE-1);					//update the tail pointer of the circular buffer after reading data
	}
	
	return i;
 }

 /*
 * FunctionDescription: Called when the device file is written.
 * Function Arguments: 
 *  filep - pointer to the file structure for the opened device.
 *  buffer - pointer to the user buffer containing data to be written.
 *  cnt - number of bytes to write.
 *  off - pointer to the file offset.
 * Function Return values:
 *  Number of bytes written on success, negative error code on failure.
 */
ssize_t test_write(struct file *filep,const char __user *buffer,size_t cnt,loff_t *off)
{
	int ret,i,m;
	printk("\n in write function\n");
	m=min(CIRC_SPACE(cbuf.head,cbuf.tail,SIZE),(int)cnt);	//calculate the number of bytes to write based on the circular buffer state and requested count
	for(i=0;i<m;i++)
	{
		ret=copy_from_user(cbuf.buf+cbuf.head,buffer+i,1);	//copy data from user space to kernel space
	 	if(ret)
	 	{ 
	   		printk("\n not copied\n");
	   		return -ENOMEM;
	 	}
		printk("\n copied from user %c\n",cbuf.buf[cbuf.head]);	//print the data being copied from user space to kernel space
		cbuf.head=(cbuf.head+1) & (SIZE-1);						//update the head pointer of the circular buffer after writing data
	}
	wake_up(&twq);	//wake up any processes waiting on the wait queue after writing data to the circular buffer
	
	return i;
}

/*
 * FunctionDescription: Called when the timer expires. This function is registered as the callback for the timer and is responsible for handling the timer expiration event.
 * Function Arguments: 
 *  None.
 * Function Return values: 
 */
static int __init test_init(void)
{
	int ret;
	devno = MKDEV(42,0);	//create a device number for the character device using the MKDEV macro with major number 42 and minor number 0
	ret=register_chrdev_region(devno,1,"test");
	if(ret)
	{
	 	printk("\n error in allocating device\n");
	 	return ret;
	}
	cdev_init(&_cdev,&fops);			//initialize the character device structure and link it to the file operations structure
	cbuf.buf=kmalloc(SIZE,GFP_KERNEL);	//allocate memory for the circular buffer using kmalloc with the specified size and GFP_KERNEL flag for kernel memory allocation
	init_waitqueue_head(&twq);			//initialize the wait queue head for the wait queue used in the driver
	if(!cbuf.buf)
	{
		printk("\nallocation failed\n");
		unregister_chrdev_region(devno,1);	//unregister the device number region if memory allocation for the circular buffer fails
		return -ENOMEM;
	}
	ret = cdev_add(&_cdev,devno,1);	//add the character device to the kernel and make it available for use with the specified device number and count
	if(ret)
	{
	 	printk("\n error in adding to kernel\n");
	 	return ret;
	}
	printk("\n we are in init function\n");
	return 0;
}

/*
 * FunctionDescription: Module cleanup routine that is called when the module is removed. This function is responsible for cleaning up the resources allocated by the module, such as unregistering the character device and freeing the allocated memory for the circular buffer.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  None.
 */
static void __exit test_exit(void)
{
	printk("\n in exit function\n");
	cdev_del(&_cdev);					//delete the character device from the kernel, making it unavailable for use
	kfree(cbuf.buf);					//free the allocated memory for the circular buffer
	unregister_chrdev_region(devno,1);	//unregister the device number region allocated for the character device, making the device number available for other drivers

}

module_init(test_init);	//register the init function to be called when the module is loaded
module_exit(test_exit);	//register the exit function to be called when the module is removed
