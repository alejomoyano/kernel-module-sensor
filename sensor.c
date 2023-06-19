#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");

/* Variables for device and device class */
static dev_t dev_sensor;
static struct class *class_sensor;
static struct cdev cdev_sensor;

int signal = 17;

#define DRIVER_NAME "gsensors"
#define DRIVER_CLASS "gsensors_class"

/**
 * @brief Read data out of the buffer
 */
static ssize_t sensor_driver_read(struct file *File, char *user_buffer, size_t len, loff_t *offs)
{
    int to_copy, not_copied, delta;
    char tmp[3] = " \n";

    /* amount of data to copy */
    to_copy = min(len, sizeof(tmp));

    /* read value of button */
    printk("GPIO %d: %d\n", signal, gpio_get_value(signal));
    tmp[0] = gpio_get_value(signal) + '0';

    /* copy data to user */
    not_copied = copy_to_user(user_buffer, &tmp, to_copy);

    /* calculate data */
    delta = to_copy - not_copied;

    return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t sensor_driver_write(struct file *File, const char *user_buffer, size_t len, loff_t *offs)
{
    int to_copy, not_copied, delta;
    char value;

    to_copy = min(len, sizeof(value));
    not_copied = copy_from_user(&value, user_buffer, to_copy);

    printk("Se escribio en el driver\n");

    switch(user_buffer){
        case '17':
            signal = 17;
            break;
        case '4':
            signal = 4;
            break;
        default:
            printk("GPIO no disponible. Ingrese 17 o 4.\n");
            break;
    }

    /* calculate data */
    delta = to_copy - not_copied;

    return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int sensor_driver_open(struct inode *device_file, struct file *instance)
{
    printk("Driver abierto!\n");
    return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int sensor_driver_close(struct inode *device_file, struct file *instance)
{
    printk("Driver cerrado!\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = sensor_driver_open,
    .release = sensor_driver_close,
    .read = sensor_driver_read,
    .write = sensor_driver_write};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void)
{
    printk("Montando el driver...\n");

    if (alloc_chrdev_region(&dev_sensor, 0, 1, DRIVER_NAME) < 0)
    {
        printk("Device Nr. could not be allocated!\n");
        return -1;
    }
    printk("Major: %d y Minor: %d\n", MAJOR(dev_sensor), MINOR(dev_sensor));

    if ((class_sensor = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL)
    {
        printk("Device class can not be created!\n");
        return -1;
    }

    /* create device file */
    if (device_create(class_sensor, NULL, dev_sensor, NULL, DRIVER_NAME) == NULL)
    {
        printk("Can not create device file!\n");
        return -1;
    }

    /* dev file */
    cdev_init(&cdev_sensor, &fops);

    /* Regisering device to kernel */
    if (cdev_add(&cdev_sensor, dev_sensor, 1) == -1)
    {
        printk("Registering of device to kernel failed!\n");
        return -1;
    }

    /* GPIO 4 */
    if (gpio_request(4, "rpi-gpio-4"))
    {
        printk("Can not allocate GPIO 4\n");
        return -1;
    }
    if (gpio_direction_input(4))
    {
        printk("Can not set GPIO 4 to output!\n");
        return -1;
    }

    /* GPIO 17 */
    if (gpio_request(17, "rpi-gpio-17"))
    {
        printk("Can not allocate GPIO 17\n");
        return -1;
    }
    if (gpio_direction_input(17))
    {
        printk("Can not set GPIO 17 to input!\n");
        return -1;
    }

    return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void)
{
    gpio_free(17);
    gpio_free(4);
    cdev_del(&cdev_sensor);
    device_destroy(class_sensor, dev_sensor);
    class_destroy(class_sensor);
    unregister_chrdev_region(dev_sensor, 1);
    printk("Driver desmontado.\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);
