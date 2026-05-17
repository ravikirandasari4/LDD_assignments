#include<linux/init.h>
#include<linux/module.h>

/*
 * FunctionDescription: Module initialization routine for the HelloWorld module that prints a message when the module is loaded.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static int hello_init(void)
{
    printk("\nHelloWorld\n");   //print a message when the module is loaded
    return 0;
}

/*
 * FunctionDescription: Module cleanup routine for the HelloWorld module that prints a goodbye message when the module is removed.
 * Function Arguments: 
 *  None.
 * Function Return values:
 *  0 on success, negative error code on failure.
 */
static void hello_exit(void)
{
    printk("\nGoodBye\n");    //print a message when the module is removed
}

module_init(hello_init);    //register the init function
module_exit(hello_exit);    //register the exit function
