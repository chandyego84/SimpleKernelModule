#define LINUX

#include <linux/module.h> /*Needed by all modules*/
#include <linux/init.h> /*Needed for all macros*/
#include <linux/printk.h> /*Needed for pr_info()*/
#include <linux/kernel.h>
#include "kmlab_given.h"

// Include headers as needed ...
/*Needed for procfs*/
/*Needed for procfs*/

/*Needed for linked list*/
/*Needed for linked list*/

/*Needed for timers*/
/*Needed for timers*/

/*Needed for work queue*/
/*Needed for work queue*/

/*Needed for spinlock*/
/*Needed for spinlock*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juego"); // Change with your lastname
MODULE_DESCRIPTION("CPTS360 Lab 3");

#define DEBUG 1

// Global variables as needed ...

/*Procfs GVs*/
/*Procfs GVs*/

/*Linked list GVs*/
/*Linked list GVs*/

/*Timer GVs*/
/*Timer GVs*/

/*Work queue GVs*/
/*Work queue GVs*/

/*Spinlock GVs*/
/*Spinlock GVs*/

/*START -- Procfs Functions*/
/*END -- Procfs Functions*/

/*START -- Linked List Functions*/
/*END -- Linked List Functions*/

/*START -- Timer Functions*/
/*END -- Timer Functions*/

/*START -- Work Queue/Spinlock Functions*/
/*END -- Work Queue/Spinlock Functions*/

// kmlab_init - Called when module is loaded
int __init kmlab_init(void)
{
   #ifdef DEBUG
   pr_info("HELLO! KMLAB MODULE LOADING\n");
   #endif
   // Insert your code here ...
   
   
   
   pr_info("KMLAB MODULE LOADED\n");
   return 0;   
}

// kmlab_exit - Called when module is unloaded
void __exit kmlab_exit(void)
{
   #ifdef DEBUG
   pr_info("GOODBYE! KMLAB MODULE UNLOADING\n");
   #endif
   // Insert your code here ...
   
   

   pr_info("KMLAB MODULE UNLOADED\n");
}

// Register init and exit functions
module_init(kmlab_init);
module_exit(kmlab_exit);
