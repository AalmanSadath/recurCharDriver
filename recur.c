#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define dev_name "recur"
#define RECUR_MAGIC 0x5b
#define RECUR_RESET _IO(RECUR_MAGIC, 0)
#define RECUR_INDEX _IOR(RECUR_MAGIC, 1, int)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aalman");
MODULE_DESCRIPTION("Output Recursive Patterns");

static int max_devices = 1;
module_param(max_devices, int, S_IRUGO);
MODULE_PARM_DESC(max_devices, "Maximum number of /dev/recur instances");

static int param_major_num = 0;
module_param(param_major_num, int, S_IRUGO);
MODULE_PARM_DESC(param_major_num, "Major number for the device (0 for dynamic allocation)");

int major_num;
static struct class *recur_class;
static struct cdev *recur_cdevs;

static int64_t *prev1;
static int64_t *prev2;
//0 - not initialized, 1 - only one initialized, 2 - two nums intitalized, 3 - negative num initialized
static int *initialized;
static int *index;

static int recur_open(struct inode *inode, struct file *file){
    printk("recur: open module");
    return 0;
}

static int recur_release(struct inode *inode, struct file *file){
    printk("recur: released module");
    return 0;
}

static ssize_t recur_read(struct file *file, char __user *buf, size_t size, loff_t *ppos){
    int minor = iminor(file->f_inode);
    int64_t next;

    if(initialized[minor]<=1){
        next=0;
    }
    else if(initialized[minor]==3){
        next = -1;  // initialized with negative
    }
    else if(index[minor]==0){
        next=prev2[minor];
    }
    else if(index[minor]==1){
        next=prev1[minor];
    }
    else{
        next = prev1[minor] + prev2[minor];

        if ((prev1[minor] > 0 && prev2[minor] > 0 && next < 0)) {
            next = -1;  // past maximum
        }
        else{
            prev2[minor] = prev1[minor];
            prev1[minor] = next;
        }
    }

    if (copy_to_user(buf, &next, sizeof(int64_t))) {
        return 0;
    }

    index[minor]++;
    pr_debug("%s%d: read %ld\n", dev_name, minor, size);
    return sizeof(int64_t);
}

static ssize_t recur_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos){
    int minor = iminor(file->f_inode);
    int64_t input;

    if (copy_from_user(&input, buf, sizeof(int64_t))) {
        printk("recur: error copying\n");
        return 0;
    }

    if(input<0){
        printk("recur: negative values are not allowed\n");
        prev1[minor] = -1;
        prev2[minor] = -1;
        initialized[minor] = 3;
        return 0;
    }

    if(initialized[minor]==0){
        initialized[minor]=1;
    }
    else if(initialized[minor]==1){
        initialized[minor]=2;
    }

    prev2[minor] = prev1[minor];
    prev1[minor] = input;
    index[minor] = 0;
    
    pr_debug("%s%d: wrote %ld\n", dev_name, minor, size);
    return sizeof(int64_t);
}

static long recur_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int minor = iminor(file->f_inode);
    int current_index;

    switch (cmd) {
        case RECUR_RESET:
            prev1[minor] = 0;
            prev2[minor] = 0;
            initialized[minor] = 0;
            index[minor] = 0;
            pr_debug("%s%d: RECUR_IOC_RST\n", dev_name, minor);
            break;

        case RECUR_INDEX:
            current_index = index[minor];
            if (copy_to_user((int __user *)arg, &current_index, sizeof(current_index))) {
                printk("recur: failed to copy index to user space for minor %d\n", minor);
                return -EFAULT;
            }
            pr_debug("%s%d: RECUR_IOC_INDEX: %d\n", dev_name, minor, current_index);
            break;

        default:
            pr_notice("%s%d: RECUR_IOC_ERR\n", dev_name, minor);
            break;
    }

    return 0;
}

static int recur_uevent(const struct device *dev, struct kobj_uevent_env *env){
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = recur_open,
    .release = recur_release,
    .read = recur_read,
    .write = recur_write,
    .unlocked_ioctl = recur_ioctl,
};

static int __init recur_init(void) {
        dev_t minor_num;
        int result;

        recur_cdevs = kmalloc_array(max_devices, sizeof(struct cdev), GFP_KERNEL);
        if (!recur_cdevs) {
            printk("recur: memory allocation for cdevs failed\n");
            return -ENOMEM;
        }

        if(param_major_num>0){
            major_num = param_major_num;
            minor_num = MKDEV(major_num, 0);
            result = register_chrdev_region(minor_num, max_devices, dev_name);
            if (result < 0) {
                printk("recur: failed to register provided major number %d\n", major_num);
                kfree(recur_cdevs);
                return result;
            }
            printk("recur: registered with provided major number %d\n", major_num);
        }    
        else{
            result=alloc_chrdev_region(&minor_num, 0, max_devices, dev_name);
            if(result<0){
                    printk("recur: failed to obtain major number.\n");
                    kfree(recur_cdevs);
                    return result;
            }

            major_num=MAJOR(minor_num);
            printk("recur: registered dynamic major number %d\n", major_num);
        }

        recur_class=class_create(dev_name);
        recur_class->dev_uevent=recur_uevent; //equivalent of chmod 666 /dev/recur*
        if(IS_ERR(recur_class)){
                printk("recur: error creating recur class\n");
                unregister_chrdev_region(minor_num, 1);
                kfree(recur_cdevs);
                return 0;
        }

        //array allocations
        prev1 = kmalloc_array(max_devices, sizeof(int64_t), GFP_KERNEL);
        prev2 = kmalloc_array(max_devices, sizeof(int64_t), GFP_KERNEL);
        initialized = kmalloc_array(max_devices, sizeof(int), GFP_KERNEL);
        index = kmalloc_array(max_devices, sizeof(int), GFP_KERNEL);

        if (!prev1 || !prev2 || !initialized || !index) {
            printk("recur: memory allocation for arrays failed\n");
            kfree(recur_cdevs);
            unregister_chrdev_region(minor_num, max_devices);
            return -ENOMEM;
        }

        for (int i = 0; i < max_devices; i++) {
            prev1[i] = 0;
            prev2[i] = 0;
            initialized[i] = 0;
            index[i] = 0;
        }

        //device inits
        for(int i=0;i<max_devices;i++){
            cdev_init(&recur_cdevs[i], &fops);
            recur_cdevs[i].owner=THIS_MODULE;

            result=cdev_add(&recur_cdevs[i], MKDEV(major_num, i), 1);
            if(result<0){
                pr_notice("Error %d adding %s%d\n", result, dev_name, MKDEV(major_num, i));
                while (i--) {
                    cdev_del(&recur_cdevs[i]);
                }
                class_destroy(recur_class);
                unregister_chrdev_region(minor_num, max_devices);
                kfree(recur_cdevs);
                return result;
            }

            device_create(recur_class, NULL, MKDEV(major_num, i), NULL, dev_name "%d", i);
        }

        pr_info("Device %s inserted\n", dev_name);
        return 0;
}

static void __exit recur_exit(void) {

    for(int i=0;i<max_devices;i++){
        device_destroy(recur_class, MKDEV(major_num, i));
        cdev_del(&recur_cdevs[i]);
    }

    class_destroy(recur_class);
    unregister_chrdev_region(MKDEV(major_num, 0), max_devices);
    kfree(recur_cdevs);
    kfree(prev1);
    kfree(prev2);
    kfree(initialized);
    kfree(index);
    pr_info("Device %s removed\n", dev_name);
}

module_init(recur_init);
module_exit(recur_exit);