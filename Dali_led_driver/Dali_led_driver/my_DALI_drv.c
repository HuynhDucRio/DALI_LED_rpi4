#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/string.h>
#include <linux/io.h>
#include <asm/io.h>


#define DALI_drv        "my_DALI_drv"	//device name
#define BCM2711_GPIO_ADDRESS 0xFE200000 // cat /proc/iomem | grep gpio
#define BCM2711_GPIO_SIZE 0xF4 // 244 Bytes
#define GPFSEL1_OFFSET 0x04
#define GPSET0_OFFSET 0x1c
#define GPCLR0_OFFSET 0x28
#define GPLEV0_OFFSET 0x34
#define GPIO_14  14   //OUT RPi4 model B (GPIO14) -> ESP32 PIN 16
#define GPIO_15  15   //IN  RPi4 model B (GPIO15) <- ESP32 PIN 17
#define DALI_OUT_PORT    GPIO_14
#define DALI_IN_PORT     GPIO_15

/*

 * DALI data transfer rate is 1200 bits per second (DALI frequency 1200Hz)
 * so 1 bit is transfered every (1/1200secs) 833.333us = 833333ns
 * Both high and low pulses (two states, phase change) have equal width,
 * which is equal to half the bit period => 416.666us = 416666ns for each state
 * (because of Manchester bi-phase coding, where two states are sent for every one bit)

 */
//#define DALI_BIT_HALF_PERIOD	416666	nanoseconds for each bit state transmission
#define DALI_BIT_HALF_PERIOD	100000000 //nanoseconds   //Modified for testing purpose and ESP32
#define DALI_STOPBIT_VAL    3	// 0011 stop bit value (no phase change)

static dev_t first_dev;         // Variable for the first device number
static struct cdev kernel_cdev; // Structure to keep track of the character device
static struct class *cl_pointer;// Class structure pointer

typedef struct manchesterBitValList_t {
  struct manchesterBitValList_t *pNext;
  uint8_t bitVal;
}manchesterBitValList_t;

static manchesterBitValList_t* pBitValRoot = NULL;

static struct hrtimer high_res_timer;
ktime_t dali_bit_half_period_time;


static unsigned int *gpio_register= NULL;

static void sendBit1(void)
{
  unsigned int* gpiosel =(unsigned int*)((char*) gpio_register + GPFSEL1_OFFSET); // GPFSEL1
  unsigned int* gpio_1_register =(unsigned int*)((char*)gpio_register + GPSET0_OFFSET); //GPSET0

  *gpiosel = (0x01 << 12);// GPIO Pin 14 is an output
  *gpio_1_register = (0x01 << DALI_OUT_PORT); // set GPIO Pin 14

}

static void sendBit0(void)
{
  unsigned int* gpio_0_register =(unsigned int*)((char*)gpio_register + GPCLR0_OFFSET); // GPCLR0

  *gpio_0_register = (0x01 << DALI_OUT_PORT); // clear GPIO Pin 14

}

// Timer restart function - used by the kernel module which needs a callback at a specific regular interval
// Sends bit if available and restarts timer if there are more bits to send in manchesterBitValList.
static enum hrtimer_restart timer_restart_func(struct hrtimer * hrtimer)
{
  ktime_t now;
  manchesterBitValList_t* pTemp = NULL;

  if(pBitValRoot != NULL)
  {
    if (pBitValRoot->bitVal == 1)
    {
	sendBit1();
	printk(KERN_INFO "Bit val OUT is %x \n", pBitValRoot->bitVal);
    }
    else if (pBitValRoot->bitVal == 0)
    {
	sendBit0();
	printk(KERN_INFO "Bit val OUT is %x \n", pBitValRoot->bitVal);
    }
    if(pBitValRoot->pNext != NULL)
    {
      pTemp = pBitValRoot;
      pBitValRoot = pTemp->pNext;
      kfree(pTemp);
      pTemp = NULL;
      now = hrtimer_cb_get_time(&high_res_timer);
      // hrtimer advances the expiration time to the next such interval "dali_bit_half_period_time"
      hrtimer_forward(&high_res_timer,now , dali_bit_half_period_time); // returns the amount of missed intervals
      return HRTIMER_RESTART; // restarted - recurring timer
    }
    else
    {
      kfree(pBitValRoot);
      pBitValRoot = NULL;
    }
  }
  return HRTIMER_NORESTART;
}


//Reads bits from GPIO defined in DALI_IN_PORT
static ssize_t RPi_dali_read( struct file* F, char *buf, size_t count, loff_t *f_pos )
{
	char GPIOString_in[2];
	unsigned int* gpiolev =(unsigned int*)((char*) gpio_register + GPLEV0_OFFSET); // GPLEV0
        if((*gpiolev & (0x01 << DALI_IN_PORT)))   //Check GPIO Pin 15
        {
          sprintf(GPIOString_in, "%1d", 1);
        } else {
          sprintf(GPIOString_in, "%1d", 0);
        }

	count = sizeof( GPIOString_in ); //count of bytes to read
	if( copy_to_user( buf, GPIOString_in, count ) )	//buffer to read data into from file F at position f_pos
		{return -EFAULT;}
	printk("State : %s\n",GPIOString_in);
	return count;
}


//Adds a logical bit of value val to the manchester list
static void dali_manchesterListAddVal(uint8_t val)
{
  manchesterBitValList_t* pTemp = kmalloc(sizeof(manchesterBitValList_t), GFP_KERNEL);
  pTemp->pNext = kmalloc(sizeof(manchesterBitValList_t), GFP_KERNEL); //pointer to normally (GFP_KERNEL flag) allocated memory inside the kernel space memory
  pTemp->pNext->pNext = NULL;
  switch(val)
    {
      case 0: // logical 0 is the transition from 1 to 0
        pTemp->bitVal = 0;
        pTemp->pNext->bitVal = 1;
        break;

      case DALI_STOPBIT_VAL:
        pTemp->bitVal = 1;
        pTemp->pNext->bitVal = 1;
        break;

      default: //if it's bigger than 0, it's a one
        pTemp->bitVal = 1;
        pTemp->pNext->bitVal = 0;
      break;

    }

  if(pBitValRoot != NULL){
    pTemp->pNext->pNext = pBitValRoot;
  }
  pBitValRoot = pTemp;
  printk(KERN_INFO "ListAddVal Bit val %x \n", pBitValRoot->bitVal); //for testing purposes

}

//Manchester encode each bit of each of the (address and data) bytes and add each one to the manchesterBitValList
static void dali_manchesterListAddByte(char byte)
{
  int i = 0;

  for(i = 0; i < 8; i++)
  {
    dali_manchesterListAddVal(byte & (0x1 << i));
  }

}

int myatoi(char *str)
{
	 int dec=0, i, len;
	 len=strlen(str);
	 for (i=0;i<len;i++)
	      dec=dec*10+(str[i]-'0');

	 return dec;

}

//Write bytes (address&data) from user space buffer into kernel manchester list for execution
//(F: file to write to, buf: buffer that should be written to manchester list, count: bytes that should be written, f_pos: indicates the file position to write from)
static ssize_t RPi_dali_write( struct file* F, const char *buf, size_t count, loff_t *f_pos )
{
 	char GPIOString_out[7]; // 7 bytes (6 for the DALI address&byte command characters, +1 for '\0')
				// e.g. a direct level command (address byte 254) for setting the arc power level at value (data byte) of 240, that will be (in DEC) 254240
 	int  iGPIOString_out[2]; // DALI address & data bytes
 	long ret; // unused, just to avoid the warning message "warning: ignoring return value of ‘copy_from_user’, declared with attribute warn_unused_result [-Wunused-result]"

        printk(KERN_INFO "User DALI command is = %s \n", buf);
	ret=copy_from_user(GPIOString_out, buf, 7); //copy a block of data (7 bytes) from user space (buf) to kernel space (GPIOString_out)
	printk(KERN_INFO "GPIOString_out = %s \n", GPIOString_out);

        iGPIOString_out[0]= (int) (myatoi(GPIOString_out)/1000);
        printk(KERN_INFO "iGPIOString_out[0] (address)= %d \n", iGPIOString_out[0]); // DALI address byte
	iGPIOString_out[1]= (int) (myatoi(GPIOString_out)%1000);
	printk(KERN_INFO "iGPIOString_out[1] (data)= %d \n", iGPIOString_out[1]); 	// DALI data byte

	printk(KERN_INFO "Executing WRITE.\n");

	// Fill the Manchester list from behind (adding a node to the list adds it to the front!)
	dali_manchesterListAddVal(DALI_STOPBIT_VAL); // val=3 = 0011 send stop bit (11, no phase change)
	dali_manchesterListAddVal(DALI_STOPBIT_VAL); // val=3 = 0011 send stop bit (11, no phase change)


	dali_manchesterListAddByte(iGPIOString_out[1]); //send data byte
	dali_manchesterListAddByte(iGPIOString_out[0]); //send address byte

	dali_manchesterListAddVal(1); // send start bit (1) 0001 (01, phase change)

        printk("start timer \n");
        //Starts the high resolution timer with time interpreted as relative to the current time "HRTIMER_MODE_REL".
        hrtimer_start(&high_res_timer, dali_bit_half_period_time, HRTIMER_MODE_REL);
        printk("timer started \n");

	return 0;
}


static int RPi_dali_open( struct inode *inode, struct file *file ){return 0;}

static int RPi_dali_close( struct inode *inode, struct file *file ){return 0;}

// File operations of the RPi DALi driver
// Functions defined by the driver that perform various operations on the device
static struct file_operations FileOps =
{
  .owner        = THIS_MODULE,
  .open         = RPi_dali_open,
  .read         = RPi_dali_read,
  .write        = RPi_dali_write,
  .release      = RPi_dali_close,
};

//runs once upon kernel module uploading
static int init_RPi_dali_driver(void)
{
	printk(KERN_INFO "RPi DALI Driver Kernel Module initialisation\n");
	// GPIOs initialisation
	gpio_register = (int*)ioremap(BCM2711_GPIO_ADDRESS,BCM2711_GPIO_SIZE);
	if(gpio_register==NULL) 
	{
	printk(KERN_ALERT "Failed to map GPIO!\n");
	return -1;
	}
	printk("GPIO mapped successful!\n");
	// Allocate a device character
	// only one device number=0 is allocated as it is only a single driver for one device so count=1.
	if(alloc_chrdev_region( &first_dev, 0, 1, DALI_drv ) <0)
	  {
	    printk( KERN_ALERT "Device Registration failed!\n" );
	    return -1;

	  }
	printk("Device Registratrion OK!\n");

	// Create a structure class pointer that can be used in later calls
	if ( (cl_pointer = class_create( THIS_MODULE->name )) == NULL ) // /sys/class/
	{
		printk( KERN_ALERT "Class creation failed!\n" );
		unregister_chrdev_region( first_dev, 1 );
		return -1;
	}
	printk("Class creation OK!\n");

	// Create device /dev/my_DALI_drv
	if( device_create( cl_pointer, NULL, first_dev, NULL, DALI_drv ) == NULL )
	{
		printk( KERN_ALERT "Device creation failed!\n" );
		class_destroy(cl_pointer);
		unregister_chrdev_region( first_dev, 1 );
		return -1;
	}
	printk("Device creation OK!\n");

	// Character device initialisation
	cdev_init( &kernel_cdev, &FileOps );
	if( cdev_add( &kernel_cdev, first_dev, 1 ) == -1)
	{
		printk( KERN_ALERT "Device addition failed!\n" );
		device_destroy( cl_pointer, first_dev );
		class_destroy( cl_pointer );
		unregister_chrdev_region( first_dev, 1 );
		return -1;
	}
	printk("Device addition OK! MAJOR: %d, MINOR: %d\n",MAJOR(first_dev),MINOR(first_dev));
	// Initialisation of high resolution timer
	// with time interpreted as relative to the current time "HRTIMER_MODE_REL".
	hrtimer_init(&high_res_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dali_bit_half_period_time = ktime_set(0, DALI_BIT_HALF_PERIOD); //0 secs , 100000000 ns
	high_res_timer.function = timer_restart_func; // this function is called upon timer expiration

	return 0;
}


// runs once upon kernel module unloading
void exit_RPi_dali_driver(void)
{
  printk(KERN_INFO "RPi DALI Driver Kernel Module exit!\n");

  if (hrtimer_cancel(&high_res_timer))
	printk(KERN_ALERT "Timer still running!\n");
  else printk(KERN_ALERT "Timer cancelled!\n");

  if (gpio_register) {
        iounmap(gpio_register);
        gpio_register = NULL;
        printk(KERN_INFO "GPIO unmapped successfully!\n");
  }

  cdev_del( &kernel_cdev );
  device_destroy( cl_pointer, first_dev );
  class_destroy( cl_pointer );
  unregister_chrdev_region( first_dev, 1 );

	printk(KERN_ALERT "Device unregistered!\n");
}

module_init(init_RPi_dali_driver);
module_exit(exit_RPi_dali_driver);

MODULE_AUTHOR("RED-TILE Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Raspberry Pi4 model B DALI Driver");
