// // passwd_module.c
// #include <linux/module.h>
// #include <linux/kernel.h>
// #include <linux/fs.h>
// #include <linux/uaccess.h>

// #define DEVICE_NAME "passwd_dev"
// #define BUF_LEN 1024

// static char stored_hash[BUF_LEN];
// static int device_open = 0;

// static ssize_t device_read(struct file *, char *, size_t, loff_t *);
// static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
// static int device_open_fn(struct inode *, struct file *);
// static int device_release_fn(struct inode *, struct file *);

// static struct file_operations fops = {
//     .read = device_read,
//     .write = device_write,
//     .open = device_open_fn,
//     .release = device_release_fn
// };

// int init_module(void) {
//     int ret = register_chrdev(91, DEVICE_NAME, &fops);
//     if (ret < 0) {
//         printk(KERN_ALERT "Registering char device failed with %d\n", ret);
//         return ret;
//     }
//     printk(KERN_INFO "Loaded passwd module\n");
//     return 0;
// }

// void cleanup_module(void) {
//     unregister_chrdev(91, DEVICE_NAME);
//     printk(KERN_INFO "Unloaded passwd module\n");
// }

// static int device_open_fn(struct inode *inode, struct file *file) {
//     if (device_open) return -EBUSY;
//     device_open++;
//     try_module_get(THIS_MODULE);
//     return 0;
// }

// static int device_release_fn(struct inode *inode, struct file *file) {
//     device_open--;
//     module_put(THIS_MODULE);
//     return 0;
// }

// static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset) {
//     copy_to_user(buffer, stored_hash, strlen(stored_hash));
//     return strlen(stored_hash);
// }

// static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off) {
//     if (len < BUF_LEN) {
//         copy_from_user(stored_hash, buff, len);
//         stored_hash[len] = '\0';
//         printk(KERN_INFO "Stored new password hash in kernel\n");
//     }
//     return len;
// }

// passwd_module.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>

#define DEVICE_NAME "passwd_dev"
#define BUF_LEN 1024
#define USER_DB_FILE "/etc/users.db"

static int device_open = 0;
static char *stored_data;
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int device_open_fn(struct inode *, struct file *);
static int device_release_fn(struct inode *, struct file *);

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open_fn,
    .release = device_release_fn
};

int init_module(void) {
    int ret = register_chrdev(91, DEVICE_NAME, &fops);
    if (ret < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", ret);
        return ret;
    }
    printk(KERN_INFO "Loaded passwd module\n");
    return 0;
}

void cleanup_module(void) {
    unregister_chrdev(91, DEVICE_NAME);
    printk(KERN_INFO "Unloaded passwd module\n");
    if (stored_data)
        kfree(stored_data);
}

static int device_open_fn(struct inode *inode, struct file *file) {
    if (device_open) return -EBUSY;
    device_open++;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release_fn(struct inode *inode, struct file *file) {
    device_open--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) {
    if (stored_data == NULL) {
        printk(KERN_ALERT "stored_data is NULL\n");
        return -EFAULT;
    }

    size_t len = strlen(stored_data);
    if (length < len) {
        printk(KERN_ALERT "Buffer size is too small for reading stored data\n");
        return -ENOMEM;
    }

    if (copy_to_user(buffer, stored_data, len)) {
        printk(KERN_ALERT "Failed to copy data to user\n");
        return -EFAULT;
    }

    return len;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
    struct file *user_file;
    loff_t pos = 0;
    char *user_data;
    int ret = 0;

    if (len < BUF_LEN) {
        user_data = kmalloc(len + 1, GFP_KERNEL);
        if (!user_data)
            return -ENOMEM;

        if (copy_from_user(user_data, buff, len)) {
            printk(KERN_ALERT "Failed to copy data from user\n");
            kfree(user_data);
            return -EFAULT;
        }

        user_data[len] = '\0';

        // Open user DB file to write
        user_file = filp_open(USER_DB_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (IS_ERR(user_file)) {
            ret = PTR_ERR(user_file);
            printk(KERN_ALERT "Failed to open file: %d\n", ret);
        } else {
            kernel_write(user_file, user_data, len, &pos);
            filp_close(user_file, NULL);
        }

        kfree(user_data);
    }

    return len;
}

MODULE_LICENSE("GPL");

