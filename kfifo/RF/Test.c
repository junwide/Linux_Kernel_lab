#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEMO_DEV_NAME 

int main()
{
	char buffer[64];
	int fd;
    int ret;
    char message[80] = "we come from china that is a very big country";
    char *r_buffer;
    size_t len = sizeof(message);
     
	fd = open("/dev/kfifodriver", O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		printf("open device %s failded\n", DEMO_DEV_NAME);
		return -1;
	}
    r_buffer = malloc(len*2);
    memset(r_buffer,0,2*len);
    ret = read(fd,r_buffer,2*len);
    printf("read %d bytes\n",ret);
    printf("Context: %s\n",r_buffer);

    ret = write(fd,message,len);
    if( ret != len)
    {
        printf("can not write on device %d ret: %d \n",fd,ret);
        return -1;
    }
    ret = write(fd,message ,len);
    if( ret != len) 
    {
        printf("can not write on device %d ret: %d \n",fd,ret);
        return -1;
    }
    memset(r_buffer,0,2*len);
    ret = read(fd,r_buffer,2*len);
    printf("read %d bytes\n",ret);
    printf("Context: %s\n",r_buffer);
    close(fd);
	return 0;
}
