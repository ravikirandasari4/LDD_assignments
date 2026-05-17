#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/circ_buf.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<linux/timer.h>
#include<linux/interrupt.h>

#define SIZE 8

dev_t devno;
struct cdev _cdev;
static struct timer_list mytimer;

//function prototypes for file operations and timer function
int test_open (struct inode *, struct file *);
int test_close (struct inode *, struct file *);
ssize_t test_read (struct file *, char __user *, size_t, loff_t *);
ssize_t test_write (struct file *, const char __user *, size_t, loff_t *);
void timer_fun(struct timer_list *t);

//file operations structure to link the system calls to the corresponding functions in the driver
struct file_operations fops={
	.open = test_open,		//pointer to the function to handle open system call
	.release = test_close,	//pointer to the function to handle close system call
	.read = test_read,		//pointer to the function to handle read system call
	.write = test_write		//pointer to the function to handle write system call
};

struct circ_buf cbuf;	//circular buffer structure to manage the data in the character device

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
	printk("\n cdev in inode of i_cdev %p\n",inodep->i_cdev);
	printk("\n cdev address in drive %p\n",&_cdev);
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
 * FunctionDescription: Called when the device file is read.
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
	m = min(CIRC_CNT(cbuf.head,cbuf.tail,SIZE),(int)cnt); //calculate the number of bytes to read based on the circular buffer state and requested count

	for(i=0;i<m;i++)
	{
		ret = copy_to_user(buffer+i,cbuf.buf+cbuf.tail,1);	//copy data from kernel space to user space
		if(ret)
		{ 
			printk("\n error copying to user\n");
			return -ENOMEM;
		}
		printk("\n copied to user %c\n",buffer[i]);
		cbuf.tail = (cbuf.tail+1) & (SIZE-1);	//update the tail pointer of the circular buffer
	}
	
	return i;
}

/*
 * FunctionDescription: Called when the device file is written.
 * Function Arguments: 
 *   filep - pointer to the file structure for the opened device.
 *   buffer - user-space buffer containing data to write.
 *   cnt - requested number of bytes to write.
 *   off - pointer to file offset (unused in this implementation).
 * Function Return values:
 *   number of bytes written on success, negative error code on failure.
 */
ssize_t test_write(struct file *filep,const char __user *buffer,size_t cnt,loff_t *off)
{
	int ret,i,m;
	printk("\n in write function\n");
	m = min(CIRC_SPACE(cbuf.head,cbuf.tail,SIZE),(int)cnt);	//calculate the number of bytes to write based on the circular buffer state and requested count

	for(i=0;i<m;i++)
	{
		ret = copy_from_user(cbuf.buf+cbuf.head,buffer+i,1);	//copy data from user space to kernel space
		if(ret)
		{ 
			printk("\n not copied\n");
			return -ENOMEM;
		}
		printk("\n copied from user %c\n",cbuf.buf[cbuf.head]);
		cbuf.head = (cbuf.head+1) & (SIZE-1);	//update the head pointer of the circular buffer
	}
	
	return i;
}

/*
 * FunctionDescription: Timer callback function that is called when the timer expires.
 * Function Arguments: 
 *  timer_list - pointer to the timer_list structure representing the timer that expired.
 * Function Return values:
 *  None.
 */
void timer_fun(struct timer_list *t)
{
	unsigned long expires = jiffies+msecs_to_jiffies(2000);	//calculate the next expiration time for the timer (2 seconds from now)
	printk("\n in timer function\n");
	mod_timer(t,expires);
}

/*
 * FunctionDescription: Module initialization routine for registering the character device and setting up the timer.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static int __init test_init(void)
{
	int ret;
	devno = MKDEV(45,0);
	ret=register_chrdev_region(devno,1,"test");	//register a range of device numbers for the character device
	if(ret)
	{
		printk("\n error in allocating device\n");
		return ret;
	}
	init_timer(&mytimer);		//initialize the timer structure
	cdev_init(&_cdev,&fops);	//initialize the character device structure and link it to the file operations
	cbuf.buf=kmalloc(SIZE,GFP_KERNEL);	//allocate memory for the circular buffer
	if(!cbuf.buf)
	{
		printk("\nallocation failed\n");
		unregister_chrdev_region(devno,1);	//unregister the device number region if memory allocation fails
		return -ENOMEM;
	}
	mytimer.expires=jiffies+msecs_to_jiffies(10000);	//set the initial expiration time for the timer (10 seconds from now)
	timer_setup(&mytimer,timer_fun,0);					//setup the timer to call the timer_fun function when it expires
	add_timer(&mytimer);								//add the timer to the kernel's timer list
	ret = cdev_add(&_cdev,devno,1);						//add the character device to the kernel, making it available for use
	if(ret)
	{
		printk("\n error in adding to kernel\n");
	 	return ret;
	}
	printk("\n we are in init function\n");
	return 0;
}

/*
 * FunctionDescription: Module cleanup routine for unregistering the character device and deleting the timer.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  None.
 */
static void __exit test_exit(void)
{
	cdev_del(&_cdev);	//delete the character device from the kernel
	kfree(cbuf.buf);	//free the allocated memory for the circular buffer
	del_timer_sync(&mytimer);	//delete the timer and ensure that it is not running
	unregister_chrdev_region(devno,1);	//unregister the device number region allocated for the character device
}


module_init(test_init);	//register the init function
module_exit(test_exit);	//register the exit function
