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

/**
 * @brief Read data out of the buffer
 */
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
    //    int to_copy, not_copied, delta;
    //    char tmp[1] = " ";

    /* amount of data to cosudpy */
    //    to_copy = min(len, sizeof(tmp));

    /* read value of button */
    //    printk("GPIO %d: %d\n", signal, gpio_get_value(signal));
    //    tmp[0] = gpio_get_value(signal) + '0';

    /* copy data to user */
    //    not_copied = copy_to_user(user_buffer, &tmp, to_copy);

    /* calculate data */
    //    delta = to_copy - not_copied;

    //    return delta;
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
    // switch (value)
    //{
    // case '17':
    //     signal = 17;
    //     break;
    // case '4':
    //     signal = 4;
    //     break;
    // default:
    //     printk("GPIO no disponible. Ingrese 17 o 4.\n");
    //     break;
    // }

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

    // pin 3 para Flying Fish
    // pin 7,8,9,10,11
    //    if (gpio_request(3, "rpi-gpio-3") && gpio_request(4, "rpi-gpio-4")
    //       && gpio_request(7, "rpi-gpio-7") && gpio_request(8, "rpi-gpio-8")
    //       && gpio_request(9, "rpi-gpio-9") && gpio_request(10, "rpi-gpio-10")
    //       && gpio_request(11, "rpi-gpio-11") && gpio_request(14, "rpi-gpio-14"))
    //   {
    // pin 3 para Flying Fish
    // pin 7,8,9,10,11
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

    /* GPIO 4 */
    // if (gpio_request(4, "rpi-gpio-4"))
    //{
    //     printk("Can not allocate GPIO 4\n");
    //     return -1;
    // }
    // if (gpio_direction_input(4))
    //{
    //     printk("Can not set GPIO 4 to output!\n");
    //     return -1;
    // }

    /* GPIO 17 */
    // if (gpio_request(17, "rpi-gpio-17"))
    //{
    //     printk("Can not allocate GPIO 17\n");
    //     return -1;
    // }
    // if (gpio_direction_input(17))
    //{
    //     printk("Can not set GPIO 17 to input!\n");
    //     return -1;
    // }

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
