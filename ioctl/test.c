#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/circ_buf.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include "myioctl.h"

#define SIZE 8

wait_queue_head_t twq;
dev_t devno;
struct cdev _cdev;
struct circ_buf cbuf;

//function prototypes for file operations and ioctl handler
int test_open (struct inode *, struct file *);
int test_close (struct inode *, struct file *);
ssize_t test_read (struct file *, char __user *, size_t, loff_t *);
ssize_t test_write (struct file *, const char __user *, size_t, loff_t *);
long test_ioctl(struct file *,unsigned cmd,unsigned long arg);


//file operations structure to link the system calls to the corresponding functions in the driver
struct file_operations fops={
	.open = test_open,
	.release = test_close,
	.read = test_read,
	.write = test_write,
	.unlocked_ioctl = test_ioctl,
};

/*
 * FunctionDescription: Handle ioctl system call for custom commands.
 * Function Arguments:
 *   filep - pointer to the file structure for the opened device.
 *   cmd - ioctl command code received from user space.
 *   arg - user-space argument pointer or value.
 * Function Return values:
 *   0 on success, -ENOTTY for unsupported commands.
 */
long test_ioctl(struct file *filep,unsigned cmd,unsigned long arg)
{ 
	int k;
	switch(cmd) 
	{
	 case CMD_1: 
	    printk("\n cmd1 ioctl invoked\n");
		break;
 
	 case CMD_GET:
 		get_user(k,(int*)arg);
	    printk("\n got %d from user\n",k);
		break;

	 case CMD_D:
	    break;

	 default:
	    printk("invalid command\n");
        return -ENOTTY;
	}		

   return 0;
}

/*
 * FunctionDescription: Called when the device file is opened.
 * Function Arguments:
 *   inodep - pointer to the inode representing the device.
 *   filep - pointer to the file structure for the opened device.
 * Function Return values:
 *   0 on success.
 */
int test_open(struct inode* inodep,struct file* filep)
{
	printk("\n in open function\n");
	//printk("\n cdev in inode of i_cdev %p\n",inodep->i_cdev);
	//printk("\n cdev address in drive %p\n",&_cdev);
	return 0;
}

/*
 * FunctionDescription: Called when the device file is closed.
 * Function Arguments:
 *   inodep - pointer to the inode representing the device.
 *   filep - pointer to the file structure for the opened device.
 * Function Return values:
 *   0 on success.
 */
int test_close(struct inode* inodep,struct file *filep)
{
	printk("\n in close function\n");
	return 0;
}

/*
 * FunctionDescription: Read data from the circular buffer into user space.
 * Function Arguments:
 *   filep - pointer to the file structure for the opened device.
 *   buffer - user-space buffer to receive data.
 *   cnt - requested number of bytes to read.
 *   off - pointer to file offset (unused in this implementation).
 * Function Return values:
 *   number of bytes read on success, negative error code on failure.
 */
ssize_t test_read(struct file *filep,char __user *buffer,size_t cnt,loff_t *off)
 {
	int ret,i,m;
	printk("\n in read function\n");

	if(filep->f_flags&O_NONBLOCK)
	{
		if(CIRC_CNT(cbuf.head,cbuf.tail,SIZE)==0)
		{ 
			return -EAGAIN;
		}
		else
		{ 
			goto next;
		}
    }
	wait_event_interruptible(twq,CIRC_CNT(cbuf.head,cbuf.tail,SIZE)>=1);
next:
	m=min(CIRC_CNT(cbuf.head,cbuf.tail,SIZE),(int)cnt);

	for(i=0;i<m;i++)
	{
		ret=copy_to_user(buffer+i,cbuf.buf+cbuf.tail,1);
	 	if(ret)
	 	{ 
	 	  printk("\n error copying to user\n");
	 	  return -ENOMEM;
	 	}
		printk("\n copied to user %c\n",buffer[i]);
		cbuf.tail=(cbuf.tail+1) & (SIZE-1);
	}
	
	return i;
 }

/*
 * FunctionDescription: Write data from user space into the circular buffer.
 * Function Arguments:
 *   filep - pointer to the file structure for the opened device.
 *   buffer - user-space data buffer to write from.
 *   cnt - number of bytes to write.
 *   off - pointer to file offset (unused in this implementation).
 * Function Return values:
 *   number of bytes written on success, negative error code on failure.
 */
ssize_t test_write(struct file *filep,const char __user *buffer,size_t cnt,loff_t *off)
{
	int ret,i,m;
	printk("\n in write function\n");
	m=min(CIRC_SPACE(cbuf.head,cbuf.tail,SIZE),(int)cnt);

	for(i=0;i<m;i++)
	{
	 	ret=copy_from_user(cbuf.buf+cbuf.head,buffer+i,1);
	 	if(ret)
	 	{ 
	 	  printk("\n not copied\n");
	 	  return -ENOMEM;
	 	}
		printk("\n copied from user %c\n",cbuf.buf[cbuf.head]);
		cbuf.head=(cbuf.head+1) & (SIZE-1);
	}
	wake_up(&twq);
	
	return i;
}


/*
 * FunctionDescription: Module initialization routine for registering the character device.
 * Function Arguments:
 *   None.
 * Function Return values:
 *   0 on success, negative error code on failure.
 */
static int __init test_init(void)
{
	int ret;
	devno = MKDEV(42,0);	//create a device number for the character device using the MKDEV macro with major number 42 and minor number 0
	ret=register_chrdev_region(devno,1,"test");	//register a range of device numbers for the character device

	if(ret)
	{
		printk("\n error in allocating device\n");
		return ret;
	}
	cdev_init(&_cdev,&fops);			//initialize the character device structure and link it to the file operations structure
	cbuf.buf=kmalloc(SIZE,GFP_KERNEL);	//allocate memory for the circular buffer using kmalloc with the specified size and GFP_KERNEL flag for kernel memory allocation
	init_waitqueue_head(&twq);			//initialize the wait queue head for the character device

	if(!cbuf.buf)
	{
		printk("\nallocation failed\n");
		unregister_chrdev_region(devno,1);	//unregister the device number region if memory allocation for the circular buffer fails
		return -ENOMEM;
	}
	ret = cdev_add(&_cdev,devno,1);		//add the character device to the kernel, making it available for use with the specified device number and count

	if(ret)
	{
		printk("\n error in adding to kernel\n");
		return ret;
	}
	printk("\n we are in init function\n");

	return 0;
}

/*
 * FunctionDescription: Module cleanup routine for unregistering the character device.
 * Function Arguments:
 *   None.
 * Function Return values:
 *   None.
 */
static void __exit test_exit(void)
{
	printk("\n in exit function\n");
	cdev_del(&_cdev);					//delete the character device from the kernel, making it unavailable for use
	kfree(cbuf.buf);					//free the allocated memory for the circular buffer
	unregister_chrdev_region(devno,1);	//unregister the device number region allocated for the character device, making the device number available for other drivers
}


module_init(test_init);	//register the init function
module_exit(test_exit);	//register the exit function
