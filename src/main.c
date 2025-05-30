#include <math.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <linux/fcntl.h>
#include <sys/signal.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <unistd.h>
#include "serial.h"
int serial_id = 0;

int main(int argc, char const *argv[])
{
    /* code */
    if (argv[2] == NULL)
    {
        printf("请输入串口名称");
        return -1;
    }
    
    

    int sts = serialInit(serial_id, argv[1]);
    if (sts < 0)
    {
        printf("串口初始化失败\n");
        return -1;
    }
    else
    {
        printf("串口初始化成功\n");
    }
    int param_id =0;
    printfParam();

  
    

    char readbuffer[256]={0};
    const char *file_path = argv[2];
    while (1)
    {
        int readn = read(sts,readbuffer, sizeof(readbuffer));
        if (readn > 0)
        {
            readbuffer[readn] = '\0';
            printf("Received: %s\n", readbuffer);
        }else if (readn < 0)
        {
            /* code */
            perror("read error");
        }
        
        

        printf("输入0退出，请输入读取参数:\n");
        
        scanf("%d", &param_id);
        if (param_id == 0)
        {
            break;
        }
        // appProcess(serial_id);
        appParamProcess(sts , param_id, file_path);

        
        usleep(100*1000);	
        /* code */
    }
    
 

    return 0;
}