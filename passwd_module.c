#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/random.h>

#define DEVICE_NAME "passwd_dev"
#define BUF_LEN 8192
#define USER_DB_FILE "/etc/users.db"
#define JSON_OUTPUT_FILE "/etc/users.json"

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

// Function to generate JSON output
static int generate_json_data(char *json_buffer, size_t buffer_size) {
    struct file *user_file;
    char *line_buffer;
    char *users_json;
    loff_t pos = 0;
    ssize_t bytes_read;
    int user_count = 0;
    char *current_line;
    char *line_start;
    char *temp_ptr;
    
    // Allocate buffers
    line_buffer = kmalloc(BUF_LEN, GFP_KERNEL);
    users_json = kmalloc(BUF_LEN * 2, GFP_KERNEL);
    
    if (!line_buffer || !users_json) {
        if (line_buffer) kfree(line_buffer);
        if (users_json) kfree(users_json);
        return -ENOMEM;
    }
    
    // Initialize JSON structure
    strcpy(json_buffer, "{\n  \"users\": [\n");
    strcpy(users_json, "");
    
    // Read user database file
    user_file = filp_open(USER_DB_FILE, O_RDONLY, 0);
    if (!IS_ERR(user_file)) {
        bytes_read = kernel_read(user_file, line_buffer, BUF_LEN - 1, &pos);
        if (bytes_read > 0) {
            line_buffer[bytes_read] = '\0';
            
            // Parse each line (format: username:hash)
            line_start = line_buffer;
            while ((current_line = strsep(&line_start, "\n")) != NULL) {
                if (strlen(current_line) > 0) {
                    temp_ptr = strchr(current_line, ':');
                    if (temp_ptr) {
                        *temp_ptr = '\0'; // Split at colon
                        
                        if (user_count > 0) {
                            strcat(users_json, ",\n");
                        }
                        
                        // Generate user data with hardcoded stats
                        strcat(users_json, "    {\n");
                        strcat(users_json, "      \"id\": ");
                        
                        // Convert user_count to string and append
                        {
                            char id_str[10];
                            int id = user_count + 1;
                            int i = 0, temp = id;
                            if (id == 0) {
                                id_str[i++] = '0';
                            } else {
                                while (temp > 0) {
                                    id_str[i++] = '0' + (temp % 10);
                                    temp /= 10;
                                }
                                // Reverse the string
                                int j;
                                for (j = 0; j < i / 2; j++) {
                                    char ch = id_str[j];
                                    id_str[j] = id_str[i - 1 - j];
                                    id_str[i - 1 - j] = ch;
                                }
                            }
                            id_str[i] = '\0';
                            strcat(users_json, id_str);
                        }
                        
                        strcat(users_json, ",\n      \"username\": \"");
                        strcat(users_json, current_line);
                        strcat(users_json, "\",\n      \"email\": \"");
                        strcat(users_json, current_line);
                        strcat(users_json, "@gmail.com\",\n");
                        strcat(users_json, "      \"loginTime\": \"2024-06-11T08:30:00Z\",\n");
                        strcat(users_json, "      \"logoutTime\": \"2024-06-11T17:45:00Z\",\n");
                        
                        // Generate random-ish session duration based on username length
                        {
                            char duration_str[10];
                            int duration = 480 + (strlen(current_line) * 15) % 200;
                            int i = 0, temp = duration;
                            while (temp > 0) {
                                duration_str[i++] = '0' + (temp % 10);
                                temp /= 10;
                            }
                            // Reverse
                            int j;
                            for (j = 0; j < i / 2; j++) {
                                char ch = duration_str[j];
                                duration_str[j] = duration_str[i - 1 - j];
                                duration_str[i - 1 - j] = ch;
                            }
                            duration_str[i] = '\0';
                            strcat(users_json, "      \"sessionDuration\": ");
                            strcat(users_json, duration_str);
                        }
                        
                        strcat(users_json, ",\n      \"device\": \"");
                        // Assign device based on user count
                        if (user_count % 3 == 0) {
                            strcat(users_json, "Desktop");
                        } else if (user_count % 3 == 1) {
                            strcat(users_json, "Mobile");
                        } else {
                            strcat(users_json, "Tablet");
                        }
                        
                        strcat(users_json, "\",\n      \"cpuUsage\": {\n");
                        // Generate CPU usage based on username
                        {
                            char cpu_str[5];
                            int cpu_avg = 30 + (strlen(current_line) * 7) % 50;
                            int i = 0, temp = cpu_avg;
                            while (temp > 0) {
                                cpu_str[i++] = '0' + (temp % 10);
                                temp /= 10;
                            }
                            int j;
                            for (j = 0; j < i / 2; j++) {
                                char ch = cpu_str[j];
                                cpu_str[j] = cpu_str[i - 1 - j];
                                cpu_str[i - 1 - j] = ch;
                            }
                            cpu_str[i] = '\0';
                            strcat(users_json, "        \"average\": ");
                            strcat(users_json, cpu_str);
                            strcat(users_json, ",\n        \"peak\": ");
                            
                            // Peak is average + 20-40
                            char peak_str[5];
                            int peak = cpu_avg + 25;
                            i = 0; temp = peak;
                            while (temp > 0) {
                                peak_str[i++] = '0' + (temp % 10);
                                temp /= 10;
                            }
                            for (j = 0; j < i / 2; j++) {
                                char ch = peak_str[j];
                                peak_str[j] = peak_str[i - 1 - j];
                                peak_str[i - 1 - j] = ch;
                            }
                            peak_str[i] = '\0';
                            strcat(users_json, peak_str);
                        }
                        
                        strcat(users_json, ",\n        \"hourly\": [42, 45, 48, 52, 55, 58, 61, 64, 67, 70, 68, 65, 62, 59, 56, 53, 50, 47, 44, 41, 38, 35, 32, 29]\n");
                        strcat(users_json, "      },\n      \"memoryUsage\": {\n");
                        
                        // Memory usage
                        {
                            char mem_str[5];
                            int mem_avg = 45 + (strlen(current_line) * 5) % 40;
                            int i = 0, temp = mem_avg;
                            while (temp > 0) {
                                mem_str[i++] = '0' + (temp % 10);
                                temp /= 10;
                            }
                            int j;
                            for (j = 0; j < i / 2; j++) {
                                char ch = mem_str[j];
                                mem_str[j] = mem_str[i - 1 - j];
                                mem_str[i - 1 - j] = ch;
                            }
                            mem_str[i] = '\0';
                            strcat(users_json, "        \"average\": ");
                            strcat(users_json, mem_str);
                            
                            char mem_peak_str[5];
                            int mem_peak = mem_avg + 20;
                            i = 0; temp = mem_peak;
                            while (temp > 0) {
                                mem_peak_str[i++] = '0' + (temp % 10);
                                temp /= 10;
                            }
                            for (j = 0; j < i / 2; j++) {
                                char ch = mem_peak_str[j];
                                mem_peak_str[j] = mem_peak_str[i - 1 - j];
                                mem_peak_str[i - 1 - j] = ch;
                            }
                            mem_peak_str[i] = '\0';
                            strcat(users_json, ",\n        \"peak\": ");
                            strcat(users_json, mem_peak_str);
                        }
                        
                        strcat(users_json, ",\n        \"hourly\": [58, 60, 62, 65, 68, 71, 74, 77, 80, 83, 81, 78, 75, 72, 69, 66, 63, 60, 57, 54, 51, 48, 45, 42]\n");
                        strcat(users_json, "      },\n      \"status\": \"active\"\n");
                        strcat(users_json, "    }");
                        
                        user_count++;
                    }
                }
            }
        }
        filp_close(user_file, NULL);
    }
    
    // Complete the JSON structure
    strcat(json_buffer, users_json);
    strcat(json_buffer, "\n  ],\n");
    strcat(json_buffer, "  \"systemMetrics\": {\n");
    strcat(json_buffer, "    \"totalUsers\": 192,\n");
    strcat(json_buffer, "    \"activeUsers\": ");
    
    // Convert user_count to string for active users
    {
        char count_str[10];
        int i = 0, temp = user_count;
        if (temp == 0) {
            count_str[i++] = '0';
        } else {
            while (temp > 0) {
                count_str[i++] = '0' + (temp % 10);
                temp /= 10;
            }
            int j;
            for (j = 0; j < i / 2; j++) {
                char ch = count_str[j];
                count_str[j] = count_str[i - 1 - j];
                count_str[i - 1 - j] = ch;
            }
        }
        count_str[i] = '\0';
        strcat(json_buffer, count_str);
    }
    
    strcat(json_buffer, ",\n    \"failedLogins\": 14,\n");
    strcat(json_buffer, "    \"successRate\": 96.2,\n");
    strcat(json_buffer, "    \"totalSessions\": 2356,\n");
    strcat(json_buffer, "    \"averageSessionDuration\": 487,\n");
    strcat(json_buffer, "    \"peakConcurrentUsers\": 1250,\n");
    strcat(json_buffer, "    \"totalDevices\": 10,\n");
    strcat(json_buffer, "    \"systemUptime\": 99.8,\n");
    strcat(json_buffer, "    \"lastUpdated\": \"2024-06-11T10:15:00Z\"\n");
    strcat(json_buffer, "  },\n");
    strcat(json_buffer, "  \"securityEvents\": [\n");
    strcat(json_buffer, "    {\n");
    strcat(json_buffer, "      \"id\": 1,\n");
    strcat(json_buffer, "      \"type\": \"successful_login\",\n");
    strcat(json_buffer, "      \"username\": \"system_user\",\n");
    strcat(json_buffer, "      \"email\": \"system@kernel.com\",\n");
    strcat(json_buffer, "      \"timestamp\": \"2024-06-11T10:12:00Z\",\n");
    strcat(json_buffer, "      \"ip\": \"192.168.1.45\",\n");
    strcat(json_buffer, "      \"device\": \"Desktop\"\n");
    strcat(json_buffer, "    }\n");
    strcat(json_buffer, "  ]\n");
    strcat(json_buffer, "}\n");
    
    kfree(line_buffer);
    kfree(users_json);
    return strlen(json_buffer);
}

int init_module(void) {
    int ret = register_chrdev(91, DEVICE_NAME, &fops);
    if (ret < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", ret);
        return ret;
    }
    printk(KERN_INFO "Loaded passwd module with JSON support\n");
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
    char *json_buffer;
    ssize_t json_length;
    struct file *output_file;
    loff_t pos = 0;

    json_buffer = kmalloc(BUF_LEN, GFP_KERNEL);
    if (!json_buffer)
        return -ENOMEM;

    // Generate JSON data
    json_length = generate_json_data(json_buffer, BUF_LEN);
    if (json_length < 0) {
        kfree(json_buffer);
        return json_length;
    }

    // Write JSON to output file
    output_file = filp_open(JSON_OUTPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (!IS_ERR(output_file)) {
        kernel_write(output_file, json_buffer, json_length, &pos);
        filp_close(output_file, NULL);
        printk(KERN_INFO "JSON data written to %s\n", JSON_OUTPUT_FILE);
    }

    // Copy to user buffer
    if (length < json_length) {
        kfree(json_buffer);
        return -EINVAL;
    }

    if (copy_to_user(buffer, json_buffer, json_length)) {
        kfree(json_buffer);
        return -EFAULT;
    }

    kfree(json_buffer);
    return json_length;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
    struct file *user_file;
    char *user_data;
    char *json_buffer;
    int ret;

    user_data = kmalloc(len + 2, GFP_KERNEL);
    if (!user_data)
        return -ENOMEM;

    if (copy_from_user(user_data, buff, len)) {
        printk(KERN_ALERT "Failed to copy data from user\n");
        kfree(user_data);
        return -EFAULT;
    }

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
    filp_close(user_file, NULL);
    
    // Generate updated JSON after writing user data
    json_buffer = kmalloc(BUF_LEN, GFP_KERNEL);
    if (json_buffer) {
        loff_t pos = 0;
        struct file *output_file;
        ssize_t json_length = generate_json_data(json_buffer, BUF_LEN);
        
        if (json_length > 0) {
            output_file = filp_open(JSON_OUTPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (!IS_ERR(output_file)) {
                kernel_write(output_file, json_buffer, json_length, &pos);
                filp_close(output_file, NULL);
                printk(KERN_INFO "Updated JSON data written to %s\n", JSON_OUTPUT_FILE);
            }
        }
        kfree(json_buffer);
    }
    
    kfree(user_data);
    return len;
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("User Authentication Module with JSON Export");
MODULE_VERSION("2.0");


// test
