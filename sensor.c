#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/string.h>

MODULE_LICENSE("GPL"); /*  Licencia del modulo */
MODULE_DESCRIPTION("Sensor");
MODULE_AUTHOR("Paul Carter's Gang");

// #define CANT_GPIO 2
#define CANT_GPIO 21
#define MAX_GPIO_NUMBER 32
#define BUF_SIZE 256

enum state
{
    low,
    high
};
enum direction
{
    in,
    out
};

static dev_t mm_sensor;
static struct class *sensor_class;
// static struct cdev c_dev;
struct device *dev_sensor;

struct raspi_gpio_dev *raspi_gpio[CANT_GPIO];

int selector = 0;

struct raspi_gpio_dev
{
    struct cdev cdev;
    struct gpio pin;
    enum state state;
    enum direction dir;
};

// TODO ->  crear las funciones de cambiar

static int sensor_open(struct inode *i, struct file *f)
{
    printk("Driver Sensores Abierto\n");

    struct raspi_gpio_dev *raspi_gpio;
    unsigned int gpio;

    gpio = iminor(i);
    printk("GPIO[%d] opened\n", gpio);
    raspi_gpio = container_of(i->i_cdev, struct raspi_gpio_dev, cdev);

    f->private_data = raspi_gpio;

    return 0;
}
static int sensor_close(struct inode *i, struct file *f)
{
    printk("Driver Sensores Cerrado\n");

    struct raspi_gpio_dev *raspi_gpio;
    unsigned int gpio;

    raspi_gpio = container_of(i->i_cdev, struct raspi_gpio_dev, cdev);
    gpio = iminor(i);
    printk("GPIO[%d] closed\n", gpio);

    return 0;
}

static ssize_t sensor_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    // int flag = 0;
    char byte;
    printk("Leyendo el sensor %d\n",selector);
    // deberia colocar en buf lo que lee del GPIO y len el size en bytes
    // while(flag){
        // dormirlo un segundo
    byte = '0' + gpio_get_value(selector);
    printk("%d\n", byte);
    put_user(byte, buf + len);
        // }
    return len; 
}
static ssize_t sensor_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    // deberia cambiar el sensor, o sea la variable selector.
    char input[BUF_SIZE];
    raw_copy_from_user(&input, buf, len);
    input[len] = '\0';
    printk("Cambiando a sensor %d\n", input);

    if (input == '1')
        selector = 2; 
    else if (input == '0')
        selector = 3;
    else
        printk("No existe el sensor seleccionado. Ingrese 0 o 1.\n");

    *off += len
    return len;
}

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = sensor_open,
        .release = sensor_close,
        .read = sensor_read,
        .write = sensor_write
    };

static int __init sensor_init(void) /* Constructor */
{
    int ret, index = 0;

    // asignacion de un major minor
    printk("Montando modulo de sensado...");

    // pedimos 2 char devices (mepa serian dos devices que se comunican char by char)
    ret = alloc_chrdev_region(&mm_sensor, 0, CANT_GPIO, "sensors");
    if (ret < 0)
    {
        return ret;
    }
    printk("<Major, Minor>: <%d, %d>\n", MAJOR(mm_sensor), MINOR(mm_sensor));

    // creamos el archivo en /sys
    sensor_class = class_create(THIS_MODULE, "chardevsensor");
    if (IS_ERR(sensor_class))
    {
        printk(KERN_ALERT "Error al crear la class device\n");
        return PTR_ERR(sensor_class);
    }

    for (int i = 0; i < MAX_GPIO_NUMBER; i++)
    {
        // if (i == 2 && i == 3)
        // {
        if (i != 0 && i != 1 && i != 5 && i != 6 &&
            i != 12 && i != 13 && i != 16 && i != 19 &&
            i != 20 && i != 21 && i != 26)
        {

            raspi_gpio[index] = kmalloc(sizeof(struct raspi_gpio_dev), GFP_KERNEL);

            if (!raspi_gpio[index])
            {
                printk("Bad kmalloc\n");
                return -ENOMEM;
            }

            if (gpio_request_one(i, GPIOF_OUT_INIT_LOW, NULL) < 0)
            {
                printk(KERN_ALERT "Error requesting GPIO %d\n", i);
                return -ENODEV;
            }

            // raspi_gpio[index]->dir = in;

            raspi_gpio[index]->dir = out;
            raspi_gpio[index]->state = low;
            raspi_gpio[index]->cdev.owner = THIS_MODULE;

            cdev_init(&raspi_gpio[index]->cdev, &fops);

            if ((ret = cdev_add(&raspi_gpio[index]->cdev, (mm_sensor + i), 1)))
            {
                printk(KERN_ALERT "Error %d adding cdev\n", ret);
                for (int i = 0; i < MAX_GPIO_NUMBER; i++)
                {
                    // if (i == 2 && i == 3)
                    // {
                    if (i != 0 && i != 1 && i != 5 && i != 6 &&
                        i != 12 && i != 13 && i != 16 && i != 19 &&
                        i != 20 && i != 21 && i != 26)
                    {
                        device_destroy(sensor_class, MKDEV(MAJOR(mm_sensor), MINOR(mm_sensor) + i));
                    }
                }
                class_destroy(sensor_class);
                unregister_chrdev_region(mm_sensor, CANT_GPIO);
                return ret;
            }

            if (device_create(sensor_class, NULL, MKDEV(MAJOR(mm_sensor), MINOR(mm_sensor) + i), NULL, "raspiGpio%d", i) == NULL)
            {
                class_destroy(sensor_class);
                unregister_chrdev_region(mm_sensor, CANT_GPIO);
                return -1;
            }

                index++;
            }
        }

    printk("Modulo de sensado montado.");

    return 0;
}

static void __exit sensor_exit(void) /* Destructor */
{
    // cdev_del(&c_dev);
    unregister_chrdev_region(mm_sensor, 1);

    for (int i = 0; i < MAX_GPIO_NUMBER; i++)
        kfree(raspi_gpio[i]);

    for (int i = 0; i < MAX_GPIO_NUMBER; i++){
        if (i != 0 && i != 1 && i != 5 && i != 6 &&
            i != 12 && i != 13 && i != 16 && i != 19 &&
            i != 20 && i != 21 && i != 26) {
                gpio_direction_output(i, 0);
                device_destroy(sensor_class, MKDEV(MAJOR(mm_sensor), MINOR(mm_sensor) + i));
                gpio_free(i);
            }
    }
    class_destroy(sensor_class);
    printk("Modulo de sensado desmontado.");
}

module_init(sensor_init);
module_exit(sensor_exit);
