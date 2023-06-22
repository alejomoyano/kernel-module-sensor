#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul Carter's Gang");

/* Variables for device and device class */
static dev_t dev_sensor;
static struct class *class_sensor;
static struct cdev cdev_sensor;

int signal = 0;

#define DRIVER_NAME "gsensors"
#define DRIVER_CLASS "gsensors_class"

static ssize_t sensor_driver_read(struct file *File, char *user_buffer, size_t len, loff_t *offs)
{
    int to_copy, not_copied, delta;
    char tmp[3] = " ";
    char binary[5] = " ";
    int decimal = 0;
    int bit = 0, counter = 0, potencia = 0, j = 0;
    int gpio[] = {5, 6, 13, 19, 26};

    /* amount of data to copy */
    switch (signal)
    {
    case 0:
        to_copy = min(len, sizeof(tmp));
        tmp[0] = gpio_get_value(12) + '0';

        break;
    case 1:
        for (j = 0; j < 5; j++)
        {
            bit = gpio_get_value(gpio[j]);
            printk("%d", bit);

            counter = j;
            potencia = 2;
            while (counter > 1)
            {
                potencia *= 2;
                counter--;
            }
            if (j == 0)
                potencia = 1;

            decimal += bit * potencia; // from binary to decimal
        }
        printk("\n");
        printk("decimal -> %d\n", decimal);
        // tmp[0] = gpio_get_value(signal) + '0';
        sprintf(tmp, "%d", decimal);
        to_copy = min(len, sizeof(tmp));
        break;
    }
    not_copied = copy_to_user(user_buffer, &tmp, to_copy);

    /* calculate data */
    delta = to_copy - not_copied;

    return delta;

    /* calculate data */
    delta = to_copy - not_copied;

    return delta;
}

static ssize_t sensor_driver_write(struct file *File, const char *user_buffer, size_t len, loff_t *offs)
{
    int to_copy, not_copied, delta;
    char value;

    to_copy = min(len, sizeof(value));
    not_copied = copy_from_user(&value, user_buffer, to_copy);

    printk("Se escribio en el driver\n");
    switch (value)
    {
    case '1':
        signal = 0;
        break;
    case '2':
        signal = 1;
        break;
    default:
        printk("Ingresar 1 o 2.\n");
        break;
    }
    delta = to_copy - not_copied;
    return delta;
}

static int sensor_driver_open(struct inode *device_file, struct file *instance)
{
    printk("Driver abierto!\n");
    return 0;
}

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
    .write = sensor_driver_write
    };

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

    if (gpio_request(5, "rpi-gpio-5") && gpio_request(6, "rpi-gpio-6") && gpio_request(13, "rpi-gpio-13") && gpio_request(19, "rpi-gpio-19") && gpio_request(26, "rpi-gpio-26") && gpio_request(12, "rpi-gpio-10"))
    {
        printk("Can not allocate GPIO 4\n");
        return -1;
    }

    if (gpio_direction_input(5) && gpio_direction_input(6) && gpio_direction_input(13) && gpio_direction_input(19) && gpio_direction_input(26) && gpio_direction_input(12))
    {
        printk("Can not allocate GPIO\n");
        return -1;
    }

    return 0;
}

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
