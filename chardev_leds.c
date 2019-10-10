#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/uaccess.h>	/* for copy_to_user */
#include <linux/cdev.h>
#include <linux/version.h> /* For LINUX_VERSION_CODE */
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/vt_kern.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("leds");
MODULE_LICENSE("GPL");

/*
 *  Prototypes
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "leds"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */


#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0
#define LEDS_BYTES 12

#define LED1_SCROLL 1
#define LED2_CAPS 4
#define LED3_NUM 2

struct tty_driver* kbd_driver= NULL;
/*
 * Global variables are declared as static, so are global within the file.
 */

dev_t start;
struct cdev* chardev=NULL;
static int Device_Open = 0;	/* Is device open?
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;		/* This will be initialized every time the
				   device is opened successfully */

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};


/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void)
{
    printk(KERN_INFO "leds: loading\n");
    printk(KERN_INFO "leds: fgconsole is %x\n", fg_console);
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return vc_cons[fg_console].d->port.tty->driver;
#else
    return vc_cons[fg_console].d->vc_tty->driver;
#endif
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask)
{
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
#else
    return (handler->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, mask);
#endif
}

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    int major;		/* Major number assigned to our device driver */
    int minor;		/* Minor number assigned to the associated character device */
    int ret;

    /* Get available (major,minor) range */
    if ((ret=alloc_chrdev_region (&start, 0, 1,DEVICE_NAME))) {
        printk(KERN_INFO "Can't allocate ledsdev_region()");
        return ret;
    }

    /* Create associated cdev */
    if ((chardev=cdev_alloc())==NULL) {
        printk(KERN_INFO "cdev_alloc() failed ");
        unregister_chrdev_region(start, 1);
        return -ENOMEM;
    }

    cdev_init(chardev,&fops);

    if ((ret=cdev_add(chardev,start,1))) {
        printk(KERN_INFO "cdev_add() failed ");
        kobject_put(&chardev->kobj);
        unregister_chrdev_region(start, 1);
        return ret;
    }

    major=MAJOR(start);
    minor=MINOR(start);

    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'sudo mknod -m 666 /dev/%s c %d %d'.\n", DEVICE_NAME, major,minor);
    printk(KERN_INFO "Remove the device file and module when done.\n");

    kbd_driver= get_kbd_driver_handler();
    set_leds(kbd_driver,ALL_LEDS_ON);

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    /* Destroy chardev */
    if (chardev)
        cdev_del(chardev);
    set_leds(kbd_driver,ALL_LEDS_OFF);
    /*
     * Unregister the device
     */
    unregister_chrdev_region(start, 1);
}

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev"
 */
static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open)
        return -EBUSY;

    Device_Open++;

    /* Initialize msg */
    //sprintf(msg, "Device opened!\n", 0);
    printk(KERN_INFO  "Device opened\n");
    /* Initially, this points to the beginning of the message */
    msg_Ptr = msg;

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    if(Device_Open)
        Device_Open--;
    return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
                           char *buffer,	/* buffer to fill with data */
                           size_t length,	/* length of the buffer     */
                           loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_to_read = length;

    /*
     * If we're at the end of the message,
     * return 0 -> end of file
     */
    if (*msg_Ptr == 0)
        return 0;

    /* Make sure we don't read more chars than
     * those remaining to read
    	 */
    if (bytes_to_read > strlen(msg_Ptr))
        bytes_to_read=strlen(msg_Ptr);

    /*
     * Actually transfer the data onto the userspace buffer.
     * For this task we use copy_to_user() due to security issues
     */
    if (copy_to_user(buffer,msg_Ptr,bytes_to_read))
        return -EFAULT;

    /* Update the pointer for the next read operation */
    msg_Ptr+=bytes_to_read;

    /*
     * The read operation returns the actual number of bytes
     * we copied  in the user's buffer
     */
    return bytes_to_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/chardev
 */
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char *local_buff = kmalloc(8*sizeof(char),GFP_KERNEL);
    int bytes_read = 0;
    int led1=0;
    int led2=0;
    int led3=0;
    int mask=0;
    int i = 0 ;

if(copy_from_user(local_buff,buff,LEDS_BYTES))
		return -EFAULT;

	//LEER Y PROCESAR CADA NUMERO/LED A ENCENDER DEL BUFFER
	set_leds(kbd_driver,ALL_LEDS_OFF);
	printk(KERN_INFO  "Antes del while\n");

    while( i < strlen(local_buff) && local_buff[i] != '\n' && local_buff[i] != '\0')
	{
	printk(KERN_INFO "Dentro del whilee\n");
    bytes_read += 4;
		switch(local_buff[i]){
			case '3':
				led3 = LED3_NUM;
				break;
			case '2':
			    led2 = LED2_CAPS;
				break;
			case '1':   
                   led1 = LED1_SCROLL;
				break;
	}
    ++i;
	}
    kfree(local_buff);
          
        mask = led1+led2+led3;
       switch(mask){
        case 0:
            set_leds(kbd_driver,0x0);
            break;
        case 1:
            //printk(KERN_INFO "se enciende %d",mask);
            set_leds(kbd_driver,0x1);
            break;
        case 2:
            set_leds(kbd_driver,0x2);
            break;
        case 3:
            set_leds(kbd_driver,0x3);
            break;
        case 4:
            set_leds(kbd_driver,0x4);
            break;
        case 5:
            set_leds(kbd_driver,0x5);
            break;
        case 6:
            set_leds(kbd_driver,0x6);
            break;
        case 7:
            set_leds(kbd_driver,0x7);
            break;
    }

	return bytes_read == 0 ? 4 : bytes_read;
}
