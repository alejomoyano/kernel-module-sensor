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

    switch (value)
    {
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

// TODO ->  crear las funciones de cambiar

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

    // asignacion de un major minor
    printk(KERN_INFO "Montando modulo de sensado...");
    ret = alloc_chrdev_region(&mm_sensor, 0, 1, "sensors");
    if (ret < 0)
    {
        printk("Device Nr. could not be allocated!\n");
        return -1;
    }
    printk(KERN_INFO "<Major, Minor>: <%d, %d>\n", MAJOR(mm_sensor), MINOR(mm_sensor));

    // creamos el archivo en /dev
    sensor_class = class_create(THIS_MODULE, "chardevsensor");
    if (IS_ERR(sensor_class))
    {
        printk("Device class can not be created!\n");
        return -1;
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

    // "pedimos" los pines
    if (gpio_request(4, "rpi-gpio-4"))
        printk("Error pidiendo el GPIO\n");

    if (gpio_request(17, "rpi-gpio-17"))
        printk("Error pidiendo el GPIO\n");

    // seteamos como entrada los pines
    if (gpio_direction_input(4))
        printk("Error seteando GPIO como entrada\n");

    if (gpio_direction_input(17))
        printk("Error seteando GPIO como entrada\n");

    return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void)
{
    cdev_del(&c_dev);
    device_destroy(sensor_class, mm_sensor);
    class_destroy(sensor_class);
    unregister_chrdev_region(mm_sensor, 1);
    printk(KERN_INFO "Desmontando modulo de sensado.");
}

module_init(ModuleInit);
module_exit(ModuleExit);
