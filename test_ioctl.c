#include "dev_ioctl.h"


#define ERR(...) fprintf(stderr, "\7" __VA_ARGS__), exit(EXIT_FAILURE)



#define __TEST_FUNC_OPEN_CLOSE 								\
	printf("name of the file to open: %s\n", DEV_PATH); 				\
	if ((dfd = open(DEV_PATH, O_RDWR)) < 0) {ERR("Open device error: %m\n");}	\
	printf("success");								\
	close(dfd); 									\
	dfd = 0;
	

#define __TEST_FUNC_READ(_BUFFER_NAME, _BUFFER_SIZE) 					\
	printf("name of the file to open: %s\n", DEV_PATH);  				\
	if ((dfd = open(DEV_PATH, O_RDWR)) < 0) {ERR("Open device error: %m\n");}	\
	char* _BUFFER_NAME = (char*)malloc(_BUFFER_SIZE);				\
	if (!_BUFFER_NAME) {ERR("Error memory alloc");}					\
											\
	if ((res = read(dfd, _BUFFER_NAME, _BUFFER_SIZE - 1)) > 0) 			\
	{										\
		_BUFFER_NAME[res] = '\0';						\
		printf("read %d bytes: %s\n", res, _BUFFER_NAME);			\
	} 										\
											\
	else if (res < 0)								\
	{										\
		free(_BUFFER_NAME); ERR("read device error%m\n");			\
	}										\
											\
	else 										\
	{										\
		printf("read end of stream\n");					 	\
	} 										\
											\
											\
	free(_BUFFER_NAME);								\
	res = 0; 									\
	close(dfd);									\
	dfd = 0; 										
	
	
	
	
	
#define __TEST_FUNC_WRITE(_BUFFER_NAME, _BUFFER_SIZE, _MESSAGE) 					\
	printf("name of the file to open: %s\n", DEV_PATH);  						\
	if ((dfd = open(DEV_PATH, O_RDWR)) < 0) {ERR("Open device error: %m\n");} 			\
	char* _BUFFER_NAME = (char*)malloc(_BUFFER_SIZE);						\
	if (strlen(_MESSAGE) > _BUFFER_SIZE) {strncpy(_BUFFER_NAME, _MESSAGE, _BUFFER_SIZE - 1);}  	\
	else {strncpy(_BUFFER_NAME, _MESSAGE, strlen(_MESSAGE));}					\
	if ((res = write(dfd, _BUFFER_NAME, _BUFFER_SIZE - 1)) > 0) 					\
	{												\
													\
		printf("write %d bytes: %s\n", res, _BUFFER_NAME);					\
	} 												\
													\
	else if (res < 0)										\
	{												\
		free(_BUFFER_NAME); ERR("write device error%m\n");					\
	}												\
	res = 0; 											\
	close(dfd);											\
	dfd = 0; 											\
	free(_BUFFER_NAME);											
																								
						
	
	
	

#define __TEST_FUNC_IOC(_STRUCT_NAME, _REQ_NAME, _BUF_NAME) 						\
	printf("name of the file to open: %s\n", DEV_PATH); 						\
	if ((dfd = open(DEV_PATH, O_RDWR)) < 0) {ERR("Open device error: %m\n");} 			\
	struct _STRUCT_NAME* _BUF_NAME = (struct _STRUCT_NAME*)malloc(sizeof(struct _STRUCT_NAME)); 	\
	if (ioctl(dfd, _REQ_NAME, &_BUF_NAME)) {free(_BUF_NAME); ERR("%d error: %m\n", _REQ_NAME);} 	\
	fprintf(stdout, (char*)&_BUF_NAME); 								\
	close(dfd); 											\
	dfd = 0;											\
	free(_BUF_NAME);										\


int main(int argc, char** argv)
{
	int dfd;
	ssize_t res;
	
	if (argc != 2)
	{
		ERR("enter --help for help information\n");
	}
	
	if (strcmp(argv[1], "--help") == 0)
	{
		printf("help informations: \n1 - test open | close, 2 - test read, 3 - test write, 4 - test ioctl\n");
		return EXIT_SUCCESS;
	}
	
	if (strcmp(argv[1], "1") == 0)
	{
		printf("open | close test device\n");
		__TEST_FUNC_OPEN_CLOSE;
		__TEST_FUNC_OPEN_CLOSE;
		__TEST_FUNC_OPEN_CLOSE;
		goto out;
	
	}
	
	
	
	if (strcmp(argv[1], "2") == 0)
	{
		printf("read test device\n");
		__TEST_FUNC_READ(BUF_R1, 512);
		__TEST_FUNC_READ(BUF_R2, 512);
		__TEST_FUNC_READ(BUF_R3, 512);
		goto out;
	
	}
	
	
	if (strcmp(argv[1], "3") == 0)
	{
		printf("write test device\n");
		__TEST_FUNC_WRITE(BUF_W1, 512, "Hello from userspace!");
		__TEST_FUNC_WRITE(BUF_W2, 512, "121312321321321321313");
		__TEST_FUNC_WRITE(BUF_W3, 512, "test test test test");
		goto out;
	
	}
	
	
	if (strcmp(argv[1], "4") == 0)
	{
		printf("ioctl test device\n");
		__TEST_FUNC_IOC(_TRANSFER_STRING, IOC_GET_STRING, BUF1);
		__TEST_FUNC_IOC(_TRANSFER_STRING, IOC_GET_STRING, BUF2);
		__TEST_FUNC_IOC(_TRANSFER_STRING, IOC_GET_STRING, BUF3);
		
		__TEST_FUNC_IOC(_GET_MESSAGE, IOC_PRINTK_MES, BUF4);
		__TEST_FUNC_IOC(_GET_MESSAGE, IOC_PRINTK_MES, BUF5);
		__TEST_FUNC_IOC(_GET_MESSAGE, IOC_PRINTK_MES, BUF6);
		
		__TEST_FUNC_IOC(_BIG_BUFFER_ALL_PARAM, IOC_GET_ALL_PARAM, BUF7);
		__TEST_FUNC_IOC(_BIG_BUFFER_ALL_PARAM, IOC_GET_ALL_PARAM, BUF8);
		__TEST_FUNC_IOC(_BIG_BUFFER_ALL_PARAM, IOC_GET_ALL_PARAM, BUF9);
		goto out;
	
	}
	
	else
	{
		ERR("enter --help for help information\n");
	}
	
	
out:
	return EXIT_SUCCESS;
}








