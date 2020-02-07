#include <linux/init.h>

#include <linux/module.h>

#include <linux/device.h>

#include <linux/kernel.h>

#include <linux/fs.h>

#include <asm/uaccess.h>

#include <linux/uaccess.h>

#include <linux/slab.h>

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "contact"
#define BUF_LEN 256

static int Major;
static int Device_Open = 0;

static char message[256] = {0};
static short size_of_message = 0;

static struct file_operations fops = {.read = device_read,
                                      .write = device_write,
                                      .open = device_open,
                                      .release = device_release};

int init_module(void) {
  Major = register_chrdev(0, DEVICE_NAME, &fops);

  if (Major < 0) {
    printk(KERN_ALERT "Registering char device failed with %d\n", Major);
    return Major;
  }
  printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
  return SUCCESS;
}

void cleanup_module(void) { unregister_chrdev(Major, DEVICE_NAME); }

static int device_open(struct inode *inode, struct file *file) {

  if (Device_Open)
    return -EBUSY;

  Device_Open++;
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
  Device_Open--; /* We're now ready for our next caller */

  /*
   * Decrement the usage count, or else once you opened the file, you'll
   * never get get rid of the module.
   */
  module_put(THIS_MODULE);

  return 0;
}

static ssize_t device_read(struct file *filep, char *buffer, size_t len,
                           loff_t *offset) {
  int error_count = 0;
  // copy_to_user has the format ( * to, *from, size) and returns 0 on success
  error_count = copy_to_user(buffer, message, size_of_message);

  if (error_count == 0) { // if true then have success
    printk(KERN_INFO "EBBChar: Sent %d characters to the user\n",
           size_of_message);
    return (size_of_message = 0);
  } else {
    printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n",
           error_count);
    return -EFAULT;
  }
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t len,
                            loff_t *offset) {
  int error_count = 0;
  char *kbuffer;
  kbuffer = kmalloc(len, GFP_KERNEL);
  error_count = copy_from_user(kbuffer, buffer, len);
  sprintf(message, "%s(%zu letters)", kbuffer, len);
  size_of_message = strlen(message);
  printk(KERN_INFO "Received %zu characters from the user\n", len);
  kfree(kbuffer);
  return len;
}
