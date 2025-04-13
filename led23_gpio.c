#include <linux/module.h>           /* Defines functions such as module_init/module_exit */
#include <linux/gpio.h>             /* Defines functions such as gpio_request/gpio_free */
#include <linux/platform_device.h>  /* For platform devices */
#include <linux/gpio/consumer.h>    /* For GPIO Descriptor */
#include <linux/of.h>               /* For DT */  
#include <linux/fs.h>     
#include <linux/device.h> 
#include <linux/cdev.h>    
#include <linux/slab.h>     
#include <linux/uaccess.h>  

#define DRIVER_AUTHOR "tuantnt08@gmail.com"
#define DRIVER_DESC   " Pi4 gpio test devicetree"

#define LOW     0
#define HIGH    1

typedef struct {
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
    int size;
} m_foo_dev;

m_foo_dev mdev;

char kernel_buff[50];
struct gpio_desc *gpio23;

/*function prototype*/
static int my_pdrv_probe(struct platform_device *pdev);
static int my_pdrv_remove(struct platform_device *pdev);

static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset);

static const struct of_device_id gpiod_dt_ids[] = {
    { .compatible = "Leds_Test", },
    { /* sentinel */ }
};

/* platform driver */
static struct platform_driver led23_drv = {
    .probe = my_pdrv_probe,
    .remove = my_pdrv_remove,
    .driver = {
        .name = "descriptor-based",
        .of_match_table = of_match_ptr(gpiod_dt_ids),
        .owner = THIS_MODULE,
    },
};

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .write      = m_write,
    .open       = m_open,
    .release    = m_release,
};


/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    pr_info("System call open() called...!!!\n");
    return 0;
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    pr_info("System call close() called...!!!\n");
    return 0;
}


/* This function will be called when we write the Device file */
static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
{
    int ret;

    /* Copy the buffer from user */
    ret = copy_from_user(kernel_buff, user_buf, size);
    if (ret)
    {
        pr_err("%s - copy_from_user failed\n", __func__);
        return -EFAULT;
    }

    /* If the string is "clear", clear the display */
    if ( kernel_buff[0] == '1')
    {
        gpiod_set_value(gpio23, HIGH);
    }
    else {
        gpiod_set_value(gpio23, LOW);
    }

    /* Make the buffer empty */
    memset(kernel_buff, 0, sizeof(kernel_buff));
    
    return size;
}

static int __init Init(void)
{
     if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "my-cdev-led23"))
    {
        pr_err("ERROR: Can not make number device\n");
        return -1;
    }
    pr_info("Kho tao : bat dau\n");
    
    pr_info("Major : %d    Minor : %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));
    
    if ((mdev.m_class = class_create(THIS_MODULE, "my_class_led23")) == NULL){
        pr_err("ERROR: Can not create class\n");
        goto rm_dev_num;
    }
    pr_info("Init : Khoi tao class create\n");

    if (device_create(mdev.m_class, NULL, mdev.dev_num, NULL, "my_led23_device") == NULL)
    {
        pr_err("ERROR: Can not create device\n");
        goto rm_class;

    }
    pr_info("Init : Khoi tao device create\n");

    cdev_init(&mdev.m_cdev, &fops);
    pr_info("Init : Khoi tao Cdev Init\n");
    if (cdev_add(&mdev.m_cdev, mdev.dev_num, 1) < 0)
    {
        pr_err("ERROR: Can not add device\n");
        goto rm_dev_num;
    }
    pr_info("Init : Khoi tao Cdev add\n");

    platform_driver_register(&led23_drv);
    pr_info("register platform device\n");

    pr_info("Khoi tao ket thuc\n");

    return 0;

rm_dev_num:
    unregister_chrdev_region(mdev.dev_num,1);
rm_class:
    class_destroy(mdev.m_class);

    return -1;
}

static int my_pdrv_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    gpio23 = gpiod_get(dev, "led23", GPIOD_OUT_LOW);

    pr_info("Led23 probe function .........\n");
    pr_info("%s - %d", __func__, __LINE__);
    return 0;
}

static void __exit Exit(void) {
    pr_info("Start Huy\n");
    platform_driver_unregister(&led23_drv);
    pr_info("unregister platform device\n");

    cdev_del(&mdev.m_cdev); 
    pr_info("Exit: huy cdev");

    device_destroy(mdev.m_class, mdev.dev_num);
    pr_info("Exit: huy device_destroy\n");

    class_destroy(mdev.m_class);
    pr_info("Exit: check huy class_destroy\n");


    unregister_chrdev_region(mdev.dev_num, 1);
    pr_info("Exit: check huy number\n");

    pr_info("End Huy\n");
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    gpiod_set_value(gpio23, LOW);
    gpiod_put(gpio23);

    pr_info("Led23 remove function .........\n");
    pr_info("%s - %d", __func__, __LINE__);
    return 0;
}

module_init(Init);
module_exit(Exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.0");