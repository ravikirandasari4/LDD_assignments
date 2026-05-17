#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/circ_buf.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include<linux/types.h>
#include<linux/kernel.h>
#include "myioctl.h"

#define SIZE 8
#define MAJORNO 60
#define NMINORS 3
#define STARTMINOR 0

dev_t devno,tmpdevno;	//device number for the character device

//structure to represent each device
struct myDev{
	struct cdev _cdev;  	//character device structure for this device
	struct circ_buf cbuf;	//circular buffer structure to manage the data in the character device
	wait_queue_head_t twq;	//wait queue for this device
}_devs[NMINORS];

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
 *  filep - pointer to the file structure representing the device file.
 *  cmd - the ioctl command.
 *  arg - the argument for the ioctl command.
 * Function Return values:
 *  0 on success, -ENOTTY for invalid command.
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
 		    get_user(k,(int*)arg);	//retrieve an integer value from user space and store it in variable k
	        printk("\n got %d from user\n",k);
	    break;

	 case CMD_D:
			printk("\n cmd d ioctl invoked\n");
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
 *  inodep - pointer to the inode structure representing the device file.
 *  filep - pointer to the file structure representing the device file.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
int test_open(struct inode* inodep,struct file* filep)
{
	struct myDev *mdev=container_of(inodep->i_cdev,struct myDev,_cdev);	//get the pointer to the myDev structure corresponding to the opened device file using container_of macro
	filep->private_data = mdev;	//store the pointer to the myDev structure in the private_data field of the file structure for later use in read/write/ioctl operations
	printk("\n in open function\n");
	//printk("\n cdev in inode of i_cdev %p\n",inodep->i_cdev);
	//printk("\n cdev address in drive %p\n",&_cdev);
	
	return 0;
}

/*
 * FunctionDescription: Called when the device file is closed.
 * Function Arguments: 
 *  inodep - pointer to the inode structure representing the device file.
 *  filep - pointer to the file structure representing the device file.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
int test_close(struct inode* inodep,struct file *filep)
{
	printk("\n in close function\n");
	return 0;
}

/*
 * FunctionDescription: Called when the device file is read.
 * Function Arguments: 
 *  filep - pointer to the file structure representing the device file.
 *  buffer - pointer to the user space buffer where the data will be copied.
 *  cnt - number of bytes to read.
 *  off - pointer to the file offset.
 * Function Return values:
 *  Number of bytes read on success, negative error code on failure.
 */
ssize_t test_read(struct file *filep,char __user *buffer,size_t cnt,loff_t *off)
 {
	int ret,i,m;
	printk("\n in read function\n");
	struct myDev *tdev=filep->private_data;
	if(filep->f_flags&O_NONBLOCK)
	{
		if(CIRC_CNT(tdev->cbuf.head,tdev->cbuf.tail,SIZE)==0)
		{ 
			return -EAGAIN;
		}
		else
		{ 
   	    	goto next;
		}
    }
	//wait until there is data in the circular buffer to read
	wait_event_interruptible(tdev->twq,CIRC_CNT(tdev->cbuf.head,tdev->cbuf.tail,SIZE)>=1);
next:
	m = min(CIRC_CNT(tdev->cbuf.head,tdev->cbuf.tail,SIZE),(int)cnt);	//calculate the number of bytes to read based on the circular buffer state and requested count
	for(i=0;i<m;i++)
	{
		ret=copy_to_user(buffer+i,tdev->cbuf.buf+tdev->cbuf.tail,1);	//copy data from kernel space to user space
	    if(ret)
		{ 
	        printk("\n error copying to user\n");
	        return -ENOMEM;
	    }
		printk("\n copied to user %c\n",tdev->cbuf.buf[tdev->cbuf.tail]);	//print the data being copied to user space
		tdev->cbuf.tail=(tdev->cbuf.tail+1) & (SIZE-1);						//update the tail pointer of the circular buffer
	}
	
	return i;
 }

 /*
 * FunctionDescription: Called when the device file is written.
 * Function Arguments: 
 *  filep - pointer to the file structure representing the device file.
 *  buffer - pointer to the user space buffer containing the data to be written.
 *  cnt - number of bytes to write.
 *  off - pointer to the file offset.
 * Function Return values:
 *  Number of bytes written on success, negative error code on failure.
 */
ssize_t test_write(struct file *filep,const char __user *buffer,size_t cnt,loff_t *off)
{
	int ret,i,m;
	printk("\n in write function\n");
	struct myDev *pdev=filep->private_data;	//get the pointer to the myDev structure corresponding to the opened device file from the private_data field of the file structure
	m=min(CIRC_SPACE(pdev->cbuf.head,pdev->cbuf.tail,SIZE),(int)cnt);	//calculate the number of bytes to write based on the circular buffer state and requested count
	for(i=0;i<m;i++)
	{
		ret = copy_from_user(pdev->cbuf.buf+pdev->cbuf.head,buffer+i,1); //copy data from user space to kernel space
	 	if(ret)
	 	{ 
	 	  printk("\n not copied\n");
	 	  return -ENOMEM;
	 	}
		if(ret)
		{ 
			printk("\n not copied\n");
			return -ENOMEM;
		}
		printk("\n copied from user %c\n",pdev->cbuf.buf[pdev->cbuf.head]); //print the data being copied from user space
		pdev->cbuf.head=(pdev->cbuf.head+1) & (SIZE-1);
	}
	wake_up(&pdev->twq);	//wake up any processes waiting on the wait queue for this device after writing data to the circular buffer
	
	return i;
}

/*
 * FunctionDescription: test_init is the module initialization routine that registers the character device and initializes the circular buffer and wait queue for each device.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static int __init test_init(void)
{
	int ret;
	dev_t tmpdev;
	devno = MKDEV(23,0);	//device number for the character device
	int i,j; 
	ret = register_chrdev_region(devno,NMINORS,"multi");	//register a range of device numbers for the character device with the specified major number and number of minor numbers
	if(ret)
	{
		printk("\n error in allocating device\n");
		return ret;
	}
	for(i=0;i<NMINORS;i++)
	{
		cdev_init(&_devs[i]._cdev,&fops);
	 	_devs[i].cbuf.buf = kmalloc(SIZE,GFP_KERNEL);	//allocate memory for the circular buffer of each device
	 	if(!_devs[i].cbuf.buf)
		{
			printk("\nallocation failed\n");
			goto cbuff_err;
		}
       	init_waitqueue_head(&_devs[i].twq);				//initialize the wait queue for each device
	}

	for(j=0;j<NMINORS;j++)
	{
		tmpdevno = MKDEV(MAJOR(devno),MINOR(devno)+j);	//calculate the device number for each minor device based on the major number and minor number
		ret = cdev_add(&_devs[j]._cdev,tmpdevno,1);		//add the character device to the kernel and associate it with the calculated device number
		if(ret)
		{
			printk("\n error in adding to kernel\n");
			goto cdev_err;
	  	}
	}
	printk("\n we are in init function\n");
	return 0;

cdev_err:	//cleanup routine to remove any added character devices in case of error during initialization
	for(--j ; j>=0 ; j--)
	    cdev_del(&_devs[i]._cdev);


cbuff_err:	//cleanup routine to free any allocated circular buffer memory and unregister the device numbers in case of error during initialization
	for(--i ; i>=0 ; i--)
	{
		kfree(_devs[i].cbuf.buf);					//free the allocated memory for the circular buffer of each device
		unregister_chrdev_region(devno,NMINORS);	//unregister the range of device numbers allocated for the character device
		return -ENOMEM;
	}
}

/*
 * FunctionDescription: test_exit is the module cleanup routine that unregisters the character device and frees the allocated resources for each device when the module is removed.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  None.
 */
static void __exit test_exit(void)
{
	int i;
	printk("\n in exit function\n");
	for(i=0;i<NMINORS;i++)
	{
	  kfree(_devs[i].cbuf.buf);		//free the allocated memory for the circular buffer of each device
	  cdev_del(&_devs[i]._cdev);	//delete the character device from the kernel for each device
	}		
    unregister_chrdev_region(devno,NMINORS);	//unregister the range of device numbers allocated for the character device

}

module_init(test_init);	//register the init function
module_exit(test_exit);	//register the exit function
