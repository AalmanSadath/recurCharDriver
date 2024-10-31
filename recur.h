#ifndef RECUR_H
#define RECUR_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>

#define DEV_NAME "recur"
#define RECUR_MAGIC 0x5b
#define RECUR_RESET _IO(RECUR_MAGIC, 0)
#define RECUR_INDEX _IOR(RECUR_MAGIC, 1, int)

extern int max_devices;
extern int param_major_num;
extern int major_num;
extern struct class *recur_class;
extern struct cdev *recur_cdevs;

extern int64_t *prev1;
extern int64_t *prev2;
extern int *initialized;
extern int *index;

int recur_open(struct inode *inode, struct file *file);
int recur_release(struct inode *inode, struct file *file);
ssize_t recur_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
ssize_t recur_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos);
long recur_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int recur_uevent(const struct device *dev, struct kobj_uevent_env *env);
int __init recur_init(void);
void __exit recur_exit(void);

#endif // RECUR_H