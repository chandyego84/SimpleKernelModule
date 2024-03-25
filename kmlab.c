#define LINUX

#include <linux/module.h> /*Needed by all modules*/
#include <linux/init.h> /*Needed for all macros*/
#include <linux/printk.h> /*Needed for pr_info()*/
#include <linux/kernel.h> /*We're doing kernel work*/
#include "kmlab_given.h"

/*Needed for procfs*/
#include <linux/proc_fs.h> /*For doing proc fs*/
#include <linux/uaccess.h> /*For copy_from_user, get_user, put_user*/
#include <linux/version.h>
#include <linux/string.h> /*For memset*/
/*Needed for procfs*/

/*Needed for linked list*/
#include <linux/types.h>
#include <linux/list.h>
/*Needed for linked list*/

/*Needed for timers*/
#include <linux/timer.h>
#include <linux/jiffies.h>
/*Needed for timers*/

/*Needed for work queue*/
#include <linux/workqueue.h>
/*Needed for work queue*/

/*Needed for spinlock*/
#include <linux/spinlock.h>
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
    long int cpu_value; // periodically updated via kernel timer
    int pid;
};

struct list_head process_list; // head of linked list storing each process and info
/*Linked list GVs*/

/*Timer GVs*/
static struct timer_list my_timer;
int time_interval = 5000;
/*Timer GVs*/

/*Work queue GVs*/
static struct workqueue_struct *queue = NULL;
static struct work_struct work;
/*Work queue GVs*/

/*Spinlock GVs*/
static DEFINE_SPINLOCK(sl);
/*Spinlock GVs*/

/*START -- Linked List Functions*/
// Adds a node to the tail of the linked list
int add_node(int pid) {
    struct ll_struct *new_node = kmalloc((size_t) (sizeof(struct ll_struct)), GFP_KERNEL);
    if (!new_node) {
        printk(KERN_ERR "Memory allocation for new node failed\n");
        return 1;
    }

    new_node->pid = pid;
    new_node->cpu_value = 0;
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

    printk(KERN_ERR "Could not find node with pid %d to delete\n", pid);
    return 1;
}

// Delete the linked list
void delete_list(void) {
    struct ll_struct *entry, *tmp;

    list_for_each_entry_safe(entry, tmp, &process_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }
}

// Update cpu values in the list
void update_list_cpu(void) {
    struct ll_struct *entry, *tmp;
    int ret = 0;
    unsigned long cpu_time = 0;

    list_for_each_entry_safe(entry, tmp, &process_list, list) {
        ret = get_cpu_use(entry->pid, &cpu_time);
        if (ret == -1) {
            // process has terminated -> remove from list
            delete_node(entry->pid);
            printk(KERN_INFO "Process %d has terminated and removed from list\n", entry->pid);
        }
        else {
            entry->cpu_value = cpu_time;
        }
    }
}

// Traverse the linked list, displaying each node info
void show_list(void) {
    struct ll_struct *entry = NULL;

    list_for_each_entry(entry, &process_list, list) {
        printk(KERN_INFO "%d: %ld\n", entry->pid, entry->cpu_value);
    }
}
/*END -- Linked List Functions*/

/*START -- Procfs Functions*/
/* Called when /proc file is read
 * Prints a list of all registered PIDs in the system and their corresponding userspace CPU times
 * Format:
 * PID1: <CPU Time of PID1>
 * PID2: <CPU Time of PID2>
 * */
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer,
                            size_t buffer_length, loff_t *offset) {
    struct ll_struct *entry = NULL;
    char temp[50] = "";
    int len = sizeof(procfs_buffer);
    ssize_t ret = len;

    // clear internal buffer
    memset(&procfs_buffer[0], 0, sizeof(procfs_buffer));

    // store info of each process in the list to the buffer
    list_for_each_entry(entry, &process_list, list) {
        snprintf(temp, sizeof(procfs_buffer), "PID %d: CPU Time %ld\n", entry->pid, entry->cpu_value);
        strcat(procfs_buffer, temp);
        memset(&temp[0], 0, sizeof(temp));
    }

    if (*offset >= sizeof(procfs_buffer)) {
        return -EBUSY;
    }
    if (copy_to_user(buffer, procfs_buffer, sizeof(procfs_buffer))) {
        return -EBUSY;
    }
    else {
        printk(KERN_INFO "procfile read /proc/%s/%s:\n%s", PROCFS_DIR, PROCFS_NAME, procfs_buffer);
        *offset += len;
    }

    return ret;
}

/* Called when /proc file is written
 * When registering PID:
 * PID written from user app -> proc filesystem entry
 * Proc File Write Callback: Create a node with PID and add to linked list
 * */
static ssize_t procfile_write(struct file *file, const char __user *buff,
                             size_t len, loff_t *off) {
    int pid = 0;
    int ret;
    procfs_buffer_size = len;

    // clear internal buffer
    memset(&procfs_buffer[0], 0, sizeof(procfs_buffer));

    if (procfs_buffer_size > PROCFS_MAX_SIZE) {
        // cap the userspace input if it exceeds max buffer size
        procfs_buffer_size = PROCFS_MAX_SIZE;
    }

    if (copy_from_user(procfs_buffer, buff, procfs_buffer_size)) {
        // could not copy data from userspace
        return -EFAULT;
    }

    // convert string in buffer to integer
    ret = kstrtoint(procfs_buffer, 10, &pid);
    if (ret < 0) {
        printk(KERN_ERR "Invalid data from userspace: %s\n", procfs_buffer);
    }
    if (add_node(pid)) {
        // could not add node to list
        return -ENOMEM;
    }

    printk(KERN_INFO "Added process for PID %d to the list\n", pid);

    return procfs_buffer_size;
}

static const struct proc_ops proc_file_fops = {
        .proc_read = procfile_read,
        .proc_write = procfile_write,
};
/*END -- Procfs Functions*/

/*START -- Timer Functions*/
void timer_callback(struct timer_list *timer) {
    printk(KERN_INFO "This line is printed every %d ms\n", time_interval);
    schedule_work(&work); // see work_handler for details

    // this makes a periodic timer
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(time_interval));
}
/*END -- Timer Functions*/

/*START -- Work Queue/Spinlock Functions*/
// Updates the CPU times of processes in the list
static void work_handler(struct work_struct *data) {
    printk(KERN_INFO "KM_WQ work handler function\n");

    unsigned long flags;
    spin_lock_irqsave(&sl, flags);

    printk(KERN_INFO "locked spinlock - updating the CPU times of processes\n");
    // this code uses 100% CPU time
    update_list_cpu();

    spin_unlock_irqrestore(&sl, flags);
    printk(KERN_INFO "unlocked spinlock");
}
/*END -- Work Queue/Spinlock Functions*/

// kmlab_init - Called when module is loaded
int __init kmlab_init(void)
{
   #ifdef DEBUG
   pr_info("KMLAB MODULE LOADING\n");
   #endif

    /*PROCFS ACTIONS*/
    proc_dir = proc_mkdir(PROCFS_DIR, NULL);
    if (proc_dir == NULL) {
        pr_alert("Error: Could not initialize dir /proc/%s\n", PROCFS_DIR);
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", PROCFS_DIR);

    proc_file = proc_create(PROCFS_NAME, 0666, proc_dir, &proc_file_fops);
   if (proc_file == NULL) {
       pr_alert("Error: Could not initialize proc file /proc/%s/%s\n", PROCFS_DIR, PROCFS_NAME);
       return -ENOMEM;
   }
   pr_info("/proc/%s/%s created\n", PROCFS_DIR, PROCFS_NAME);

   /*LIST ACTIONS*/
    INIT_LIST_HEAD(&process_list);

    /*TIMER ACTIONS*/
    // set up timer for initial use
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(time_interval));

    /*WORK QUEUE ACTIONS*/
    queue = alloc_workqueue("KM_WQ", WQ_UNBOUND, 1);
    INIT_WORK(&work, work_handler);

    pr_info("KMLAB MODULE LOADED\n");
   return 0;   
}

// kmlab_exit - Called when module is unloaded
void __exit kmlab_exit(void)
{
   #ifdef DEBUG
   pr_info("KMLAB MODULE UNLOADING\n");
   #endif

    /*PROCFS ACTIONS*/
    proc_remove(proc_file);
    pr_info("file /proc/%s/%s removed\n", PROCFS_DIR, PROCFS_NAME);
    proc_remove(proc_dir);
    pr_info("dir /proc/%s removed\n", PROCFS_DIR);

    /*LIST ACTIONS*/
    delete_list();

    /*TIMER ACTIONS*/
    del_timer(&my_timer);

    /*WORK QUEUE ACTIONS*/
    destroy_workqueue(queue);

   pr_info("KMLAB MODULE UNLOADED\n");
}

// Register init and exit functions
module_init(kmlab_init);
module_exit(kmlab_exit);
