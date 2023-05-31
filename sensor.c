#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");    /*  Licencia del modulo */
MODULE_DESCRIPTION("Sensor");
MODULE_AUTHOR("Paul Carter's Gang");


static dev_t mm_sensor;
static struct class *sensor_class;
static struct cdev c_dev;
struct device *dev_sensor;

// TODO ->  crear las funciones de fops

static int sensor_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: open()\n");
    return 0;
}
static int sensor_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: close()\n");
    return 0;
}
static ssize_t sensor_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver: read()\n");
    return 0;
}
static ssize_t sensor_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver: write()\n");
    return len;
}

static struct file_operations fops =
    {
        .owner = THIS_MODULE,
        .open = sensor_open,
        .release = sensor_close,
        .read = sensor_read,
        .write = sensor_write
    };

static int __init sensor_init(void) /* Constructor */
{
    int ret;

    // asignacion de un major minor
    printk(KERN_INFO "Montando modulo de sensado...");
    ret = alloc_chrdev_region(&mm_sensor, 0, 1, "sensors");
    if (ret < 0)
    {
        return ret;
    }
    printk(KERN_INFO "<Major, Minor>: <%d, %d>\n", MAJOR(mm_sensor), MINOR(mm_sensor));

    // creamos el archivo en /dev
    sensor_class = class_create(THIS_MODULE, "chardevsensor");
    if (IS_ERR(sensor_class))
    {
        printk(KERN_ALERT "Error al crear la class device\n");
        return PTR_ERR(sensor_class);
    }

    dev_sensor = device_create(sensor_class, NULL, mm_sensor, NULL, "sensors");
    if (IS_ERR(dev_sensor))
    {
        class_destroy(sensor_class);
        unregister_chrdev_region(mm_sensor, 1);
        return PTR_ERR(dev_sensor);
    }

    cdev_init(&c_dev, &fops);
    ret = cdev_add(&c_dev, mm_sensor, 1);
    if (ret < 0)
    {
        device_destroy(sensor_class, mm_sensor);
        class_destroy(sensor_class);
        unregister_chrdev_region(mm_sensor, 1);
        return ret;
    }

    return 0;
}

static void __exit sensor_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(sensor_class, mm_sensor);
    class_destroy(sensor_class);
    unregister_chrdev_region(mm_sensor, 1);
    printk(KERN_INFO "Desmontando modulo de sensado.");
}

module_init(sensor_init);
module_exit(sensor_exit);

