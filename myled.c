#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>

MODULE_AUTHOR("Ryunosuke Tabata");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("COPYING");
MODULE_VERSION("0.0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
        char c;
        if(copy_from_user(&c, buf,sizeof(char)))
        return -EFAULT;

//      printk(KERN_INFO "receive %d\n", c);


        if(c == '0'){
                gpio_base[10] = 1 << 25;
        //      gpio_base[10] = 1 << 24;
        }
        else if(c == '1'){
                gpio_base[7] = 1 << 25;
        //      gpio_base[7] = 1 << 24;
        }

        return 1;
}

static int __init init_mod(void)
{
        int retval;
        retval = alloc_chrdev_region(&dev, 0, 1, "myled");
        if(retval < 0){
                printk(KERN_ERR "alloc_chrdev_region failed.\n");
                return retval;
        }
        printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));

        cdev_init(&cdv, &led_fops);
        retval = cdev_add(&cdv, dev, 1);
        if(retval < 0){
                printk(KERN_ERR "cdev_add failed. major:%d, minor:%d\n" ,MAJOR(dev),MINOR(dev));
                return retval;
        }


        cls = class_create(THIS_MODULE, "myled");
        if(IS_ERR(cls)){
                printk(KERN_ERR "class_create failed.");
                return PTR_ERR(cls);
        }
        device_create(cls, NULL, dev,NULL, "myled%d",MINOR(dev));

        gpio_base = ioremap_nocache(0xfe200000, 0xA0);

        const u32 Led = 25;
        const u32 Index = Led/10;
        const u32 Shift = (Led%10)*3;
        const u32 Mask = ~(0x7<<Shift);
        gpio_base[Index] = (gpio_base[Index] & Mask ) | (0x1 << Shift);

        return 0;
}


static void __exit cleanup_mod(void)
{
        cdev_del(&cdv);
        device_destroy(cls, dev);
        class_destroy(cls);
        cdev_del(&cdv);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "%s is unloaded. major:%d\n",__FILE__,MAJOR(dev));
}


module_init(init_mod);
module_exit(cleanup_mod);