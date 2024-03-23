#define LINUX

#include <linux/module.h> /*Needed by all modules*/
#include <linux/init.h> /*Needed for all macros*/
#include <linux/printk.h> /*Needed for pr_info()*/
#include <linux/kernel.h> /*We're doing kernel work*/
#include "kmlab_given.h"

/*Needed for procfs*/
#include <linux/proc_fs.h> /*For doing proc fs*/
#include <linux/uaccess.h> /*For copy_from_user*/
#include <linux/version.h>
#include <linux/string.h> /*For memset*/
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
MODULE_AUTHOR("Juego");
MODULE_DESCRIPTION("CPTS360 Lab 3");

#define DEBUG 1

// Global variables as needed ...

/*Procfs GVs*/
#define PROCFS_DIR "kmlab"
#define PROCFS_NAME "status"
#define KMLAB_MAX_SIZE 1024

// Struct holds information about our proc dir and proc file
static struct proc_dir_entry *proc_file, *proc_dir;
static char proc_buffer[KMLAB_MAX_SIZE] = "";
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
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer,
                            size_t buffer_length, loff_t *offset) {

}

static ssize_t procfile_write(struct file *file, const char __user *buff,
                             size_t len, loff_t *off) {

}

static const struct proc_ops proc_file_fops = {
        .proc_read = procfile_read,
        .proc_write = procfile_write,
};

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
   pr_info("KMLAB MODULE LOADING\n");
   #endif

    // Create proc filesystem entries
    proc_dir = proc_mkdir(PROCFS_DIR, NULL);
    if (proc_dir == NULL) {
        pr_alert("Error: Could not initialize dir /proc/%s\n", PROCFS_DIR);
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", PROCFS_DIR);

    proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops);
   if (proc_file == NULL) {
       pr_alert("Error: Could not initialize proc file /proc/%s/%s\n", PROCFS_DIR, PROCFS_NAME);
       return -ENOMEM;
   }
   pr_info("/proc/%s/%s created\n", PROCFS_DIR, PROCFS_NAME);
   
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
