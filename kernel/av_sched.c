#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AV Scheduler");
MODULE_DESCRIPTION("Autonomous Vehicle Task Scheduler with Controlled Logging");

#define PROCFS_MAX_SIZE     1024
#define PROC_CONTROL_FILE   "av_control"
#define PROC_STATUS_FILE    "av_sched_status"

static struct task_struct *brake_thread;
static struct task_struct *path_thread;
static struct task_struct *comm_thread;

static DECLARE_WAIT_QUEUE_HEAD(brake_wq);
static DECLARE_WAIT_QUEUE_HEAD(path_wq);
static DECLARE_WAIT_QUEUE_HEAD(comm_wq);

static bool brake_triggered = false;
static bool brake_active = false;
static bool path_active = false;
static bool comm_active = false;

static struct proc_dir_entry *control_entry;
static struct proc_dir_entry *status_entry;

static DEFINE_MUTEX(state_lock);

int brake_thread_fn(void *data) {
    while (!kthread_should_stop()) {
        wait_event_interruptible(brake_wq, brake_triggered);
        if (brake_triggered) {
            mutex_lock(&state_lock);
            brake_active = true;
            path_active = false;
            comm_active = false;
            mutex_unlock(&state_lock);

            printk(KERN_INFO "[av_sched] Emergency braking triggered!\n");
            msleep(500);

            mutex_lock(&state_lock);
            brake_active = false;
            brake_triggered = false;
            mutex_unlock(&state_lock);

            wake_up(&path_wq);
            wake_up(&comm_wq);
        }
    }
    return 0;
}

int path_thread_fn(void *data) {
    while (!kthread_should_stop()) {
        wait_event_interruptible(path_wq, !brake_triggered);
        mutex_lock(&state_lock);
        path_active = true;
        mutex_unlock(&state_lock);

        msleep(200);

        mutex_lock(&state_lock);
        path_active = false;
        mutex_unlock(&state_lock);

        if (!brake_triggered)
            wake_up(&comm_wq);
    }
    return 0;
}

int comm_thread_fn(void *data) {
    while (!kthread_should_stop()) {
        wait_event_interruptible(comm_wq, !brake_triggered);
        mutex_lock(&state_lock);
        comm_active = true;
        mutex_unlock(&state_lock);

        msleep(150);

        mutex_lock(&state_lock);
        comm_active = false;
        mutex_unlock(&state_lock);

        if (!brake_triggered)
            wake_up(&path_wq);
    }
    return 0;
}

static ssize_t control_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) {
    char input[PROCFS_MAX_SIZE];
    if (count >= PROCFS_MAX_SIZE) return -EINVAL;
    if (copy_from_user(input, buffer, count)) return -EFAULT;

    input[count] = '\0';
    if (strncmp(input, "object", 6) == 0) {
        mutex_lock(&state_lock);
        brake_triggered = true;
        mutex_unlock(&state_lock);
        wake_up(&brake_wq);
    }
    return count;
}

static ssize_t status_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char status[512];
    int len = 0;

    if (*ppos > 0) return 0;

    mutex_lock(&state_lock);
    len += snprintf(status + len, sizeof(status) - len,
                    "Braking Task: %s\n",
                    brake_active ? "Active" : "Inactive");
    len += snprintf(status + len, sizeof(status) - len,
                    "Path Planning Task: %s\n",
                    path_active ? "Active" : "Inactive");
    len += snprintf(status + len, sizeof(status) - len,
                    "Communication Task: %s\n",
                    comm_active ? "Active" : "Inactive");
    mutex_unlock(&state_lock);

    if (copy_to_user(buf, status, len)) return -EFAULT;
    *ppos = len;
    return len;
}

static const struct file_operations control_fops = {
    .owner = THIS_MODULE,
    .write = control_write,
};

static const struct file_operations status_fops = {
    .owner = THIS_MODULE,
    .read = status_read,
};

static int __init av_sched_init(void) {
    printk(KERN_INFO "[av_sched] Module loaded.\n");

    control_entry = proc_create(PROC_CONTROL_FILE, 0666, NULL, &control_fops);
    status_entry = proc_create(PROC_STATUS_FILE, 0444, NULL, &status_fops);

    brake_thread = kthread_run(brake_thread_fn, NULL, "brake_thread");
    path_thread = kthread_run(path_thread_fn, NULL, "path_thread");
    comm_thread = kthread_run(comm_thread_fn, NULL, "comm_thread");

    wake_up(&path_wq);
    wake_up(&comm_wq);

    return 0;
}

static void __exit av_sched_exit(void) {
    printk(KERN_INFO "[av_sched] Module unloaded.\n");

    kthread_stop(brake_thread);
    kthread_stop(path_thread);
    kthread_stop(comm_thread);

    proc_remove(control_entry);
    proc_remove(status_entry);
}

module_init(av_sched_init);
module_exit(av_sched_exit);

