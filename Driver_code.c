#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/gpio.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#define PIN_COUNT 12
static int gpio_num[PIN_COUNT]={12,13,14,15,16,17,18,19,20,21,22,23};
static dev_t dev;
static struct cdev gpio_cdev;
static struct class *dev_class;
static struct gpio_desc *gpio_desc[PIN_COUNT];
uint8_t *kernel_buf;

struct gpio gpio_pins[] = {
    { .gpio = 12, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO12" },
    { .gpio = 13, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO13" },
    { .gpio = 14, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO14" },
    { .gpio = 15, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO15" },
    { .gpio = 16, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO16" },
    { .gpio = 17, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO17" },
    { .gpio = 18, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO18" },
    { .gpio = 19, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO19" },
    { .gpio = 20, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO20" },
    { .gpio = 21, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO21" },
    { .gpio = 22, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO22" },
    { .gpio = 23, .flags = GPIOF_OUT_INIT_LOW, .label = "GPIO23" },
    
};


static int gpio_open(struct inode *inode,struct file *file)
{
	pr_info("multi_gpio:Device opened\n");
	return 0;
}

static int gpio_release(struct inode * inode, struct file *file)
{
	pr_info("Multi gpio: Device closed successfully\n");
	return 0;
}

static ssize_t gpio_write(struct file *file, const char __user *buf,size_t len, loff_t *off)
{
	
	if(len > PIN_COUNT)
		return -EINVAL;
	if(copy_from_user(kernel_buf,buf,len))
		return -EINVAL;
	kernel_buf[len]='\0';

	for(int i=0;i<PIN_COUNT;i++)
	{
		if(kernel_buf[i]=='1')
			gpiod_set_value(gpio_desc[i],1);
		else if(kernel_buf[i]=='0')
			gpiod_set_value(gpio_desc[i],0);
		else
			pr_info("Multi gpio:Invalid input value for gpio:%d", gpio_num[i]);
	}
	return len;
}

static ssize_t gpio_read(struct file *file,char __user *buf, size_t len,loff_t *off)
{

	uint8_t  gpio_state[PIN_COUNT]={0,0,0};
	if(len<1)
		return 0;
	for(int i=0;i<PIN_COUNT;i++)
	{
	gpio_state[i] = gpiod_get_value(gpio_desc[i]);
	kernel_buf[i]=gpio_state[i];
	}

	if(copy_to_user(buf,kernel_buf,len)>0)
	{
		pr_err("Error: Data not Read properly..!\n");
		return -EFAULT;
	}
	pr_info("GPIO Pin State read successfully\n");
	return len;
}



static struct file_operations fops={
 
	.owner=THIS_MODULE,
	.open=gpio_open,
	.release=gpio_release,
	.write=gpio_write,
	.read=gpio_read
};

static int __init multi_gpio_init(void)
{
	int ret;

	if((ret=alloc_chrdev_region(&dev,0,1,"Multiple_GPIO"))<0)
	{
		pr_err("multi_gpio: failed to allocate character device region\n");
		unregister_chrdev_region(dev,1);
		return ret;
	}
	pr_info("Multi gpio: Major:%d Minor:%d",MAJOR(dev),MINOR(dev));

	cdev_init(&gpio_cdev,&fops);

	if((ret=cdev_add(&gpio_cdev,dev,1))<0)
	{
		pr_err("multi_gpio:Failed to add character device\n");
		cdev_del(&gpio_cdev);
		return ret;
	}

	if(IS_ERR(dev_class=class_create(THIS_MODULE,"multi_gpio")))
	{
		pr_err("cannot create class\n");
		class_destroy(dev_class);
	}

	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"multi_gpio")))
	{
		pr_err("Cannot create device...!\n");
		device_destroy(dev_class,dev);
	}

	
	for(int i=0; i<PIN_COUNT; i++)
	{
		if(!(gpio_desc[i]=gpio_to_desc(gpio_num[i])))
		{
			pr_err("GPIO %d is not valid..!\n", gpio_num[i]);
		}		
		if(!gpio_is_valid(gpio_num[i]))
		{
			pr_err("multi_gpio: invalid gpio:%d \n",gpio_num[i]);
			return ret;
		}
	}

	if((kernel_buf=kmalloc(PIN_COUNT, GFP_KERNEL))==0)
        {
                pr_info("cannot allocate memory in kernel\n");
               class_destroy(dev_class);
        }

	if(gpio_request_array(gpio_pins,PIN_COUNT)<0)
	{
		pr_err("Multi gpio: request error\n");
	}

	for(int i=0;i<PIN_COUNT;i++)
	{
		gpio_direction_output(gpio_num[i],0);
		gpiod_export(gpio_desc[i],true);
	}
	pr_info("multi_gpio:MODULE loaded\n");
	return 0;
}

static void __exit multi_gpio_exit(void)
{
	unregister_chrdev_region(dev,1);
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&gpio_cdev);
	for(int i=0;i<PIN_COUNT;i++)
	{
		gpiod_unexport(gpio_desc[i]);
		gpio_free(gpio_num[i]);
	}
	pr_info("Device driver Remove ...Done!!\n");
}

module_init(multi_gpio_init);
module_exit(multi_gpio_exit);

MODULE_AUTHOR("DINESH");
MODULE_DESCRIPTION("GPIO MODULE");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
