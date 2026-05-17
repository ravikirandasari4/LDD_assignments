#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include "add.h"

/*
 * FunctionDescription: Module initialization routine for the avg module that prints a message when the module is loaded and calculates the average of two numbers using the add function.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static int avg_init(void)
{
	printk(KERN_ALERT "\navg init\n");	//print a message when the module is loaded
	printk(KERN_ALERT "\navg = %d\n",add(6,3)/2);	//calculate the average of 6 and 3 using the add function and print it
	return 0;
}

/*
 * FunctionDescription: Module cleanup routine for the avg module that prints a message when the module is removed.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static void avg_exit(void)
{
	printk(KERN_ALERT "\navg exit\n");	//print a message when the module is removed
}

module_init(avg_init);	//register the init function
module_exit(avg_exit);	//register the exit function
