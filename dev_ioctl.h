#ifndef __DEV_IOCTL__
#define __DEV_IOCTL__


#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/err.h>
#include <linux/parport.h>

/*

from /usr/include/asm-generic/ioctl.h

#define _IO(type,nr)            _IOC(_IOC_NONE,(type),(nr),0)
#define _IOR(type,nr,size)      _IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOW(type,nr,size)      _IOC(_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOWR(type,nr,size)     _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOR_BAD(type,nr,size)  _IOC(_IOC_READ,(type),(nr),sizeof(size))
#define _IOW_BAD(type,nr,size)  _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
#define _IOWR_BAD(type,nr,size) _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))

*/
#define IOC_MAGIC_ID 		'h'
#define IOC_GET_STRING 		_IOR(IOC_MAGIC_ID, 1, struct _TRANSFER_STRING)
#define IOC_PRINTK_MES		_IOW(IOC_MAGIC_ID, 2, struct _GET_MESSAGE)
#define IOC_GET_ALL_PARAM	_IOR(IOC_MAGIC_ID, 3, struct _BIG_BUFFER_ALL_PARAM)

#define LOG(...) 		printk(KERN_INFO "INFO: " __VA_ARGS__)
#define ERR(...) 		printk(KERN_ERR "Error: " __VA_ARGS__)
#define DBG(...) 		if (deb > 0) printk(KERN_DEBUG "DBG: " __VA_ARGS__)
#define LOGGING_ALL  		if (deb > 0) logging_all_param()
#define SUCCESS 		0
#define DEVICE_NAME 		"my_ioc_device"
#define BUFFER_SIZE 		1024

#else



#define DEV_PATH 		"/dev/my_ioc_device"
#define DEVICE_NAME 		"my_ioc_device"
#define BUFFER_SIZE 		1024

#define IOC_MAGIC_ID 		'h'
#define IOC_GET_STRING 		_IOR(IOC_MAGIC_ID, 1, struct _TRANSFER_STRING)
#define IOC_PRINTK_MES		_IOR(IOC_MAGIC_ID, 2, struct _GET_MESSAGE)
#define IOC_GET_ALL_PARAM	_IOR(IOC_MAGIC_ID, 3, struct _BIG_BUFFER_ALL_PARAM)

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


struct _TRANSFER_STRING
{
	char buffer[BUFFER_SIZE];
};

struct _GET_MESSAGE
{
	char buffer[BUFFER_SIZE];
};

struct _BIG_BUFFER_ALL_PARAM
{
	char buffer[BUFFER_SIZE * 5];
};


#endif



#ifdef __KERNEL__
struct _TRANSFER_STRING
{
	char buffer[BUFFER_SIZE];
};

struct _GET_MESSAGE
{
	char buffer[BUFFER_SIZE];
};

struct _BIG_BUFFER_ALL_PARAM
{
	char buffer[BUFFER_SIZE * 5];
};


extern ssize_t 	dev_read(struct file*, char __user*, size_t, loff_t*);
extern ssize_t 	dev_write(struct file*, const char __user*, size_t, loff_t*);
extern int 	dev_open(struct inode*, struct file*);
extern int 	dev_release(struct inode*, struct file*);
extern long 	dev_ioctl(struct file*, unsigned int, unsigned long);

extern void logging_all_param(void);


struct _GLOBAL_BUFFER
{	
	char buffer[BUFFER_SIZE * 2];
	size_t size;
};

struct _DEV
{
	struct cdev hcdev;	
	dev_t dev;
	char class_name[50];
	char module_name[50];
};

struct _DATA
{
	struct _GLOBAL_BUFFER* global_buffer;
	struct class* device_class;
	struct device* device_device;
	loff_t buffer_pos;
	struct _DEV* data_dev;
	struct file_operations fops;
};


int device_open = 0;
struct _DATA* data = NULL;
extern int __init ioctl_init(void);
extern void __exit ioctl_exit(void);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ramsun");
module_init(ioctl_init);
module_exit(ioctl_exit);
#endif // __KERNEL__

#endif // __DEV_IOCTL__
