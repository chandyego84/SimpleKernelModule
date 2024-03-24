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
#include <linux/types.h>
#include <linux/list.h>
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
#define PROCFS_MAX_SIZE 1024

// Structs hold information about our proc dir and proc file
static struct proc_dir_entry *proc_file, *proc_dir;
static char procfs_buffer[PROCFS_MAX_SIZE] = ""; // buffer used to store character for this module
static unsigned long procfs_buffer_size = 0; // size of the buffer
/*Procfs GVs*/

/*Linked list GVs*/
struct ll_struct {
    struct list_head list;
    long int cpu_value;
    int pid;
};

struct list_head process_list; // head of linked list storing each process and info

/*Linked list GVs*/

/*Timer GVs*/
/*Timer GVs*/

/*Work queue GVs*/
/*Work queue GVs*/

/*Spinlock GVs*/
/*Spinlock GVs*/

/*START -- Procfs Functions*/
/*called when /proc file is read*/
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer,
                            size_t buffer_length, loff_t *offset) {
    int len = sizeof(procfs_buffer_size);
    ssize_t ret = len;

    if (*offset >= len) {
        return 0;
    }
    if (copy_to_user(buffer, procfs_buffer, len)) {
        pr_info("copy_to_user failed\n");
        ret = 0;
    }
    else {
        pr_info("procfile read /proc/%s\n", PROCFS_NAME);
        *offset += len;
    }

    return ret;
}

/*called when /proc file is written*/
static ssize_t procfile_write(struct file *file, const char __user *buff,
                             size_t len, loff_t *off) {
    // clear internal buffer
    memset(&procfs_buffer[0], 0, sizeof(procfs_buffer));

    procfs_buffer_size = len;

    if (procfs_buffer_size > PROCFS_MAX_SIZE) {
        procfs_buffer_size = PROCFS_MAX_SIZE;
    }

    if (copy_from_user(procfs_buffer, buff, procfs_buffer_size)) {
        // copy from user failed

        return -EFAULT;
    }

    procfs_buffer[procfs_buffer_size & (PROCFS_MAX_SIZE - 1)] = '\0';
    *off += procfs_buffer_size;
    pr_info("procfile write %s\n", procfs_buffer);

    return procfs_buffer_size;
}

static const struct proc_ops proc_file_fops = {
        .proc_read = procfile_read,
        .proc_write = procfile_write,
};
/*END -- Procfs Functions*/

/*START -- Linked List Functions*/
// Adds a node to the tail of the linked list
int add_node(int pid) {
    struct ll_struct *new_node = kmalloc((size_t) (sizeof(struct ll_struct)), GFP_KERNEL);
    if (!new_node) {
        pr_info("Memory allocation for new node failed\n");
        return 1;
    }

    new_node->cpu_value = 0;
    new_node->pid = pid;
    list_add_tail(&(new_node->list), &process_list);
    return 0;
}

// Removes a node from the linked list
int delete_node(int pid) {
    struct ll_struct *entry = NULL, *n;

    list_for_each_entry_safe(entry, n, &process_list, list) {
        if (entry->pid == pid) {
            list_del(&(entry->list));
            kfree(entry);
            return 0;
        }
    }

    pr_info("Could not find node with pid %d\n", pid);
    return 1;
}

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

   // Starting linked list
    INIT_LIST_HEAD(&process_list);

    pr_info("KMLAB MODULE LOADED\n");
   return 0;   
}

// kmlab_exit - Called when module is unloaded
void __exit kmlab_exit(void)
{
   #ifdef DEBUG
   pr_info("KMLAB MODULE UNLOADING\n");
   #endif

    proc_remove(proc_file);
    pr_info("file /proc/%s/%s removed\n", PROCFS_DIR, PROCFS_NAME);
    proc_remove(proc_dir);
    pr_info("dir /proc/%s removed\n", PROCFS_DIR);
   

   pr_info("KMLAB MODULE UNLOADED\n");
}

// Register init and exit functions
module_init(kmlab_init);
module_exit(kmlab_exit);
