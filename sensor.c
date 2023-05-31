#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/gpio.h> // libreria con la que handleamos los GPIOs

MODULE_LICENSE("GPL"); /*  Licencia del modulo */
MODULE_DESCRIPTION("Sensor");
MODULE_AUTHOR("Paul Carter's Gang");

static dev_t mm_sensor;
static struct class *sensor_class;
static struct cdev c_dev;
struct device *dev_sensor;

int selector = 0;

// TODO ->  crear las funciones de cambiar

static int sensor_open(struct inode *i, struct file *f)
{
    printk("Driver Sensores Abierto\n");
    return 0;
}
static int sensor_close(struct inode *i, struct file *f)
{
    printk("Driver Sensores Cerrado\n");
    return 0;
}

static ssize_t sensor_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    int flag = 0;
    printk("Leyendo el sensor %s\n",selector);
    // deberia colocar en buf lo que lee del GPIO y len el size en bytes
    // while(flag){
        // dormirlo un segundo
    printk("%i\n",gpio_get_value(selector));
    // }
    return len; // ver esto, estaba en 0 antes
}
static ssize_t sensor_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    // deberia cambiar el sensor, o sea la variable selector.
    char input;
    copy_from_user(&input, buf, len)
        printk("Cambiando a sensor %s\n", input);

    if (input == '1')
        selector = 1; // cambiarlo por el numero del GPIO
    else if (input == '0')
        selector = 0; // a este tambien
    else
        printk("No existe el sensor seleccionado. Ingrese 0 o 1.\n");
    return len;
}

static struct file_operations fops =
    {
        .owner = THIS_MODULE,
        .open = sensor_open,
        .release = sensor_close,
        .read = sensor_read,
        .write = sensor_write};

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
