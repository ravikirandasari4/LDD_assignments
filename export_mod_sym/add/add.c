#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include "add.h"

/*
 * FunctionDescription: Module initialization routine for the add module that prints a message when the module is loaded.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static int add_init(void)
{
	printk(KERN_ALERT "\nadd init\n");	//print a message when the module is loaded
	return 0;
}

/*
 * FunctionDescription: Module cleanup routine for the add module that prints a message when the module is removed.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static void add_exit(void)
{
	printk(KERN_ALERT "\nadd exit\n");	//print a message when the module is removed
}

/*
 * FunctionDescription: Function to add two integers and return the result. This function is exported to be used by other kernel modules.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
int add(int a,int b)
{
	return(a+b);	//return the sum of a and b
}
 
EXPORT_SYMBOL(add);	//export the add function to be used by other kernel modules

module_init(add_init);	//register the init function
module_exit(add_exit);	//register the exit function
