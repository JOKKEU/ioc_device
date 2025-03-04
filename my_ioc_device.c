#include "dev_ioctl.h"

static char* name = "ioc_dev";
module_param(name, charp, 0);
static char* message = "hello from kernel space!";
module_param(message, charp, 0);
static int deb = 0;
module_param(deb, int, 0);

#define MIN_NUM(_NUM1, _NUM2)  ((_NUM1 > _NUM2) ? (_NUM2) : (_NUM1))


ssize_t dev_read(struct file* file, char __user* buffer, size_t count, loff_t* ppos)
{
	DBG("start dev_read\n");
	LOGGING_ALL;
	size_t ret_bytes = 0;
	int ret;
	
	if (!data || !data->global_buffer) { ERR("Data or global_buffer is NULL\n"); ret = -EFAULT; goto out;}
	if (*ppos >= data->buffer_pos) {  DBG("No more data to read\n"); return SUCCESS; }

	size_t bytes_to_read = MIN_NUM(count, (char *)data->buffer_pos - (char *)*ppos);
	
	LOGGING_ALL;
	DBG("bytes_to_read: %zu\n", bytes_to_read);
	DBG("ppos: %llu\n", *ppos);
    	
	if (copy_to_user(buffer, data->global_buffer->buffer + *ppos, bytes_to_read)) 
	{
		ret = -EFAULT;
		goto out;
	}

	ret_bytes += bytes_to_read;
	*ppos += bytes_to_read; 

	DBG("=== read bytes: %zu\n=== buffer_pos: %llu\n=== ppos: %llu\n",
		ret_bytes,
		data->buffer_pos,
		*ppos);



	return ret_bytes; 

out:
	ERR("in dev_read: err = %d\n", ret);
	return ret;
}

ssize_t dev_write(struct file* file, const char __user* buffer, size_t count, loff_t* ppos)
{
	DBG("=== device write === \n");
	if (!data || !data->global_buffer) { ERR("Data or global_buffer is NULL\n"); return -EFAULT; }
	if (*ppos > data->global_buffer->size - 1) {*ppos = data->global_buffer->size - 1;}	
	if (copy_from_user(data->global_buffer->buffer, buffer, count)) {ERR("Failed to copy data from user\n"); return -EFAULT;}
	data->global_buffer->buffer[count] = '\0';
	data->buffer_pos = count;
	DBG("=== device write end === \n");
	return count;
}


int dev_open(struct inode* inode, struct file* file)
{
	if (device_open)
	{
		return -EBUSY;
	}
	
	device_open++;
	
	DBG("=== device open: %d times=== \n", device_open);
	LOGGING_ALL;
	return SUCCESS;
}

int dev_release(struct inode*, struct file* file)
{
	device_open--;
	DBG("=== device close === \n");
	LOGGING_ALL;
	return SUCCESS;
}

long device_ioctl(struct file* file, unsigned int cmd, unsigned long arg) 
{

    	if (_IOC_TYPE(cmd) != IOC_MAGIC_ID) 
    	{
        	return -EINVAL; 
    	}
    	
    	switch (cmd) 
    	{
        	case IOC_GET_STRING:
        	
        		struct _TRANSFER_STRING*  transfer_string = (struct _TRANSFER_STRING*)kmalloc(sizeof (struct _TRANSFER_STRING), GFP_KERNEL);
            		if (!transfer_string)
            		{
            			ERR("Failed to allocate memory for tranfer_string\n");
            			kfree(transfer_string);
            			return -ENOMEM;
            		}
            		
           		if (copy_to_user((void __user *)arg, transfer_string->buffer, sizeof(struct _TRANSFER_STRING))) 
           		{
				ERR("Error in IOC_GET_STRING\n");
				kfree(transfer_string);
				return -EFAULT;
            		}
            		
            		kfree(transfer_string);
            		break;
            		
            		
            	case IOC_PRINTK_MES:
            		
            		struct _GET_MESSAGE*  get_message = (struct _GET_MESSAGE*)kmalloc(sizeof (struct _GET_MESSAGE), GFP_KERNEL);
            		if (!get_message)
            		{
            			ERR("Failed to allocate memory for get_message\n");
            			kfree(get_message);
            			return -ENOMEM;
            		}
            	
            		if (copy_from_user((struct _GET_MESSAGE*)arg, get_message->buffer, sizeof(struct _GET_MESSAGE)))
            		{
            			ERR("Error in IOC_PRINTK_MES\n");
            			kfree(get_message);
				return -EFAULT; 
            		}
            		
            		printk(KERN_INFO "message from userspace: %s\n", get_message->buffer);
            		kfree(get_message);
            		break;
            		
            		
            	case IOC_GET_ALL_PARAM:
            		struct _BIG_BUFFER_ALL_PARAM*  bg_bf = (struct _BIG_BUFFER_ALL_PARAM*)kmalloc(sizeof (struct _BIG_BUFFER_ALL_PARAM), GFP_KERNEL);
            		if (!bg_bf)
            		{
            			ERR("Failed to allocate memory for bg_bf\n");
            			kfree(bg_bf);
            			return -ENOMEM;
            		}
           		
            		
		    	snprintf(bg_bf->buffer, sizeof(bg_bf), "param in (struct data)\nstruct global_buffer->buffer: %lx && buffer: %s\nstruct device_class: %lx\nstruct device_device: %lx\nbuffer_pos: %lld\nstruct data_dev: %lx\ndata_devdata->dev: %d | %d : %d\ndata_dev->module_name: %s\n\ndata->fops.open: %lx\ndata->fops.release: %lx\ndata->fops.read: %lx\ndata->fops.write: %lx\ndata->fops.unlocked_ioctl: %lx\n",
		    	(uintptr_t)data->global_buffer->buffer,
		    	data->global_buffer->buffer,
		    	(data->device_class != NULL) ? (uintptr_t)data->device_class : 0,
		    	(data->device_device != NULL) ? (uintptr_t)data->device_device : 0,
		    	data->buffer_pos,
		    	(data->data_dev != NULL) ? (uintptr_t)data->data_dev : 0,
		    	data->data_dev->dev,
		    	MAJOR(data->data_dev->dev),
		    	MINOR(data->data_dev->dev),
		    	data->data_dev->module_name,
		    	(data->fops.open != NULL) ? (uintptr_t)data->fops.open : 0,
		    	(data->fops.release != NULL) ? (uintptr_t)data->fops.release : 0,
		    	(data->fops.read != NULL) ? (uintptr_t)data->fops.read : 0,
		    	(data->fops.write != NULL) ? (uintptr_t)data->fops.write : 0,
		    	(data->fops.unlocked_ioctl != NULL) ? (uintptr_t)data->fops.unlocked_ioctl : 0);
		    	
			if (copy_to_user((struct _BIG_BUFFER_ALL_PARAM*)arg, bg_bf->buffer, sizeof(bg_bf)))
			{
				ERR("Error in _BIG_BUFFER_ALL_PARAM\n");
				kfree(bg_bf);
				return -EFAULT; 
			}
			
			kfree(bg_bf);
			break;		
            	
        	default:
			return -ENOTTY;
	}

	return 0; 
}


#define __KMALLOC__(_NAME, _TYPE) \
_NAME = (struct _TYPE*)kmalloc(sizeof(struct _TYPE), GFP_KERNEL); \
do { \
	if(!_NAME) \
	{ \
		ERR("Failed to allocate memory for data structure\n"); \
		*ret = -ENOMEM; \
		return; \
	} \
} while(0)




static void struct_init(struct _DATA** data_ptr, int* ret)
{

	DBG("start struct_init\n");
	*data_ptr = (struct _DATA*)kmalloc(sizeof(struct _DATA), GFP_KERNEL);
	if (!*data_ptr)
	{
		ERR("Failed to allocate memory for data structure\n");
		*ret = -ENOMEM;
		return;
	}

	(*data_ptr)->global_buffer = (struct _GLOBAL_BUFFER*)kmalloc(sizeof(struct _GLOBAL_BUFFER), GFP_KERNEL);
	if (!(*data_ptr)->global_buffer)
	{
		ERR("Failed to allocate memory for global_buffer\n");
		kfree(*data_ptr);
		*data_ptr = NULL;
		*ret = -ENOMEM;
		return;
	}

	(*data_ptr)->data_dev = (struct _DEV*)kmalloc(sizeof(struct _DEV), GFP_KERNEL);
	if (!(*data_ptr)->data_dev)
	{
		ERR("Failed to allocate memory for data_dev\n");
		kfree((*data_ptr)->global_buffer);
		kfree(*data_ptr);
		*data_ptr = NULL;
		*ret = -ENOMEM;
		return;
	}
	DBG("struct_init 1\n");
	(*data_ptr)->global_buffer->size = 	BUFFER_SIZE * 2;
	memset((*data_ptr)->global_buffer->buffer, 0, (*data_ptr)->global_buffer->size); 
        strncpy((*data_ptr)->global_buffer->buffer, message, (*data_ptr)->global_buffer->size);
	DBG("struct_init 2\n");
	(*data_ptr)->buffer_pos = 		0;
	(*data_ptr)->data_dev->hcdev.owner = 	THIS_MODULE;
	(*data_ptr)->device_class = 		NULL;
	(*data_ptr)->device_device = 		NULL;
	DBG("struct_init 3\n");
	strncpy((*data_ptr)->data_dev->class_name, name, sizeof((*data_ptr)->data_dev->class_name) - 1);
	strncpy((*data_ptr)->data_dev->module_name, DEVICE_NAME, sizeof((*data_ptr)->data_dev->module_name) - 1);
	DBG("struct_init 4\n");
	(*data_ptr)->data_dev->dev = 		0;
	(*data_ptr)->fops.owner = 		THIS_MODULE;
	(*data_ptr)->fops.read = 		dev_read;
	(*data_ptr)->fops.write = 		dev_write;
	(*data_ptr)->fops.open = 		dev_open;
	(*data_ptr)->fops.release = 		dev_release;
	(*data_ptr)->fops.unlocked_ioctl = 	device_ioctl;
	DBG("end struct_init\n");
	return;
	
}


bool flag = false;


void logging_all_param(void)
{
	DBG("===================================================\n");
	DBG("param in (struct data)\n");
	DBG("struct global_buffer->buffer: %lx && buffer: %s\n", (data->global_buffer->buffer != NULL) ? (uintptr_t)data->global_buffer->buffer : 0 , (data->global_buffer->buffer != NULL) ? data->global_buffer->buffer : "000");
	DBG("struct device_class: %lx\n", (data->device_class != NULL) ? (uintptr_t)data->device_class : 0);
	DBG("struct device_device: %lx\n", (data->device_device != NULL) ? (uintptr_t)data->device_device : 0);
	DBG("buffer_pos: %llu\n", data->buffer_pos);
	DBG("struct data_dev: %lx\n", (data->data_dev != NULL) ? (uintptr_t)data->data_dev : 0);
	DBG("data_dev->dev: %d | %d : %d", data->data_dev->dev, MAJOR(data->data_dev->dev), MINOR(data->data_dev->dev));
	DBG("data_dev->module_name: %s\n", data->data_dev->module_name);
	DBG("data->fops.open: %lx\n", (data->fops.open != NULL) ? (uintptr_t)data->fops.open : 0);
	DBG("data->fops.release: %lx\n", (data->fops.release != NULL) ? (uintptr_t)data->fops.release : 0);
	DBG("data->fops.read: %lx\n", (data->fops.read != NULL) ? (uintptr_t)data->fops.read : 0);
	DBG("data->fops.write: %lx\n", (data->fops.write != NULL) ? (uintptr_t)data->fops.write : 0);
	DBG("data->fops.unlocked_ioctl: %lx\n", (data->fops.unlocked_ioctl != NULL) ? (uintptr_t)data->fops.unlocked_ioctl : 0);
	
	if (flag)
	{
		DBG("data->data_dev->hcdev.ops->open: %lx\n", (data->data_dev->hcdev.ops->open != NULL) ? (uintptr_t)data->data_dev->hcdev.ops->open : 0);
		DBG("data->data_dev->hcdev.ops->release: %lx\n", (data->data_dev->hcdev.ops->release != NULL) ? (uintptr_t)data->data_dev->hcdev.ops->release : 0);
		DBG("data->data_dev->hcdev.ops->read: %lx\n", (data->data_dev->hcdev.ops->read != NULL) ? (uintptr_t)data->data_dev->hcdev.ops->read : 0);
		DBG("data->data_dev->hcdev.ops->write: %lx\n", (data->data_dev->hcdev.ops->write != NULL) ? (uintptr_t)data->data_dev->hcdev.ops->write : 0);
		DBG("data->data_dev->hcdev.ops->unlocked_ioctl: %lx\n", (data->data_dev->hcdev.ops->unlocked_ioctl != NULL) ? (uintptr_t)data->data_dev->hcdev.ops->unlocked_ioctl : 0);	
	}
	
	DBG("===================================================\n");
}

int __init ioctl_init(void)
{
	int ret;
	struct_init(&data, &ret);
	LOGGING_ALL;
	if (!ret)
	{
		ERR("Can not init data\n");
		goto err;
	}
	DBG("before alloc_region\n");
	int tmp;
	ret = alloc_chrdev_region(&tmp, MINOR(data->data_dev->dev), 0, data->data_dev->module_name);
	DBG("before MKDEV\n");
	data->data_dev->dev = MKDEV(MAJOR(tmp), MINOR(data->data_dev->dev));
	LOGGING_ALL;
	if (ret < 0)
	{
		ERR("Can not register char device region\n");
		goto err;
	}
	
	cdev_init(&data->data_dev->hcdev, &data->fops);
	
	flag = true;
	
	DBG("char device init\n");
	data->data_dev->hcdev.owner = THIS_MODULE;
	ret = cdev_add(&data->data_dev->hcdev, data->data_dev->dev, 1);
	if(ret < 0) 
	{ 
		unregister_chrdev_region(data->data_dev->dev, 1);
		LOG("=== Can not add char device\n");
		goto err;
	}
	DBG("char device add\n");
	DBG("Registered char device region: %d:%d\n", MAJOR(data->data_dev->dev), MINOR(data->data_dev->dev));
	data->device_class = class_create(data->data_dev->class_name);
	if (IS_ERR(data->device_class))
	{
		unregister_chrdev(MAJOR(data->data_dev->dev), data->data_dev->module_name);
		ERR("Failed to create the device class\n");
		return PTR_ERR(data->device_class);		
	}
	LOGGING_ALL;
	data->device_device = device_create(data->device_class, NULL, data->data_dev->dev, NULL, data->data_dev->module_name);
	if (IS_ERR(data->device_device))
	{	
		class_destroy(data->device_class);
		unregister_chrdev(MAJOR(data->data_dev->dev), data->data_dev->module_name);
		ERR("Failed to create device\n");	
		return PTR_ERR(data->device_device);
	}
	LOG("================== module installed %d:%d ==================", MAJOR(data->data_dev->dev), MINOR(data->data_dev->dev));
	LOGGING_ALL;
	return ret;
	
err:
	ERR("in ioctl_init: err = %d\n", ret);
	return ret;
	
}


void __exit ioctl_exit(void)
{
    	LOGGING_ALL;

    	if (data->device_device)
    	{
        	device_destroy(data->device_class, data->data_dev->dev); 
    	}

    	if (data->device_class)
    	{
        	class_unregister(data->device_class);
        	class_destroy(data->device_class);
    	}
	cdev_del(&data->data_dev->hcdev);
    	unregister_chrdev(MAJOR(data->data_dev->dev), data->data_dev->module_name);
    	
    	kfree(data->global_buffer);
   	kfree(data->data_dev);
    	kfree(data);
    	
    	LOGGING_ALL;
    
	LOG("================== module removed ==================\n");
}


