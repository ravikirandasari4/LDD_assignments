#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/fs.h>

dev_t devno;	//device number for the character device
struct cdev _cdev;	//character device structure to represent the character device in the kernel

//function prototypes for file operations
struct file_operations fops={
	.open = test_open,		//pointer to the function to handle open system call
	.release = test_close,	//pointer to the function to handle close system call
	.read = test_read,		//pointer to the function to handle read system call
	.write = test_write		//pointer to the function to handle write system call
};

/*
 * FunctionDescription: Called when the timer expires.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static int device_init(void)
{
	int ret;
	devno = MKDEV(42,0);	//create a device number for the character device using the MKDEV macro with major number 42 and minor number 0
	ret = register_chrdev_region(devno , 1, "test");	//register a range of device numbers for the character device with the specified major number and number of minor numbers
	if(ret)
	{
	 printk("\n error in allocating device\n");
	 return ret;
	}
	cdev_init(&_cdev,&fops);	//initialize the character device structure and link it to the file operations structure
	ret = cdev_add(&_cdev,devno,1);	//add the character device to the kernel, making it available for use
	if(ret)
	{
	 printk("\n error in adding to kernel\n");
	}
	printk("\n we are in init function\n");

	return 0;
}

/*
 * FunctionDescription: Module cleanup routine for unregistering the character device.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  None.
 */
static void device_exit(void)
{
	cdev_del(&_cdev);						//delete the character device from the kernel, making it unavailable for use
	unregister_chrdev_region(devno,1);		//unregister the device number region allocated for the character device, making the device number available for other drivers
	printk("\n we are in exit function\n");

}


module_init(device_init);	//register the init function to be called when the module is loaded
module_exit(device_exit);	//register the exit function to be called when the module is removed
