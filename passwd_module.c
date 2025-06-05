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
    struct file *user_file;
    loff_t pos = 0;
    ssize_t bytes_read;
    char *kernel_buf;

    kernel_buf = kmalloc(BUF_LEN, GFP_KERNEL);
    if (!kernel_buf)
        return -ENOMEM;

    user_file = filp_open(USER_DB_FILE, O_RDONLY, 0);
    if (IS_ERR(user_file)) {
        kfree(kernel_buf);
        return PTR_ERR(user_file);
    }

    bytes_read = kernel_read(user_file, kernel_buf, BUF_LEN - 1, &pos);
    if (bytes_read < 0) {
        filp_close(user_file, NULL);
        kfree(kernel_buf);
        return bytes_read;
    }

    kernel_buf[bytes_read] = '\0';
    filp_close(user_file, NULL);

    if (copy_to_user(buffer, kernel_buf, bytes_read)) {
        kfree(kernel_buf);
        return -EFAULT;
    }

    kfree(kernel_buf);
    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
    struct file *user_file;
    char *user_data;
    int ret;

    user_data = kmalloc(len + 2, GFP_KERNEL); // extra byte for newline
    if (!user_data)
        return -ENOMEM;

    if (copy_from_user(user_data, buff, len)) {
        printk(KERN_ALERT "Failed to copy data from user\n");
        kfree(user_data);
        return -EFAULT;
    }

    // Ensure null-terminated and newline-appended string
    user_data[len] = '\n';
    user_data[len + 1] = '\0';

    user_file = filp_open(USER_DB_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(user_file)) {
        ret = PTR_ERR(user_file);
        printk(KERN_ALERT "Failed to open file: %d\n", ret);
        kfree(user_data);
        return ret;
    }

    ret = kernel_write(user_file, user_data, len + 1, &user_file->f_pos);
    if (ret < 0) {
        printk(KERN_ALERT "kernel_write failed: %d\n", ret);
    }

    filp_close(user_file, NULL);
    kfree(user_data);
    return len;
}

MODULE_LICENSE("GPL");
