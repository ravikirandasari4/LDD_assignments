#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>

static char *charvar = "ravi";	//declare a module parameter named charvar of type char pointer and initialize it with the string "ravi"
static int intvar=10;			//declare a module parameter named intvar of type int and initialize it with the value 10

module_param(charvar,charp,S_IRUGO);	//module_param macro is used to declare a module parameter named charvar of type charp (pointer to a character string) with read-only permissions for userspace (S_IRUGO)
module_param(intvar,int,S_IRUGO);		//module_param macro is used to declare a module parameter named intvar of type int with read-only permissions for userspace (S_IRUGO)

/*
 * FunctionDescription: Module initialization routine for printing the values of module parameters.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static int param_init(void)
{
	printk(KERN_ALERT "\n we are in init function\n");			//print a message when the module is loaded
	printk(KERN_ALERT "\nthe value of charvar is %s\n",charvar);//print the value of charvar parameter
	printk(KERN_ALERT "\nthe value of intvar is %d\n",intvar);	//print the value of intvar parameter
	return 0;
}

/*
 * FunctionDescription: Module cleanup routine for printing a goodbye message when the module is removed.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  None.
 */
static void param_exit(void)
{
	printk(KERN_ALERT "\n goodbye\n");
}

module_init(param_init);	//register the init function
module_exit(param_exit);	//register the init and exit functions
