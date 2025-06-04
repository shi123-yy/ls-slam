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

sem_t  *g_signal = NULL; //调试线程信号量
SerialPort *appHandle;
int fd = 0;
void pthreadStop(SerialPort *pemn) 
{
    
    pemn->threadExitFlag = 1;  // 设置全局退出标志
    
    // 等待线程结束
    pthread_join(pemn->writeThreadID, NULL);
    pthread_join(pemn->readThreadID, NULL);
    
    close(pemn->fd);
    
}
//关闭设备
void die(int x)
{
	// freeSpi();
	pthreadStop(&appHandle);

	sem_close(g_signal);     //关闭信号量
	sem_unlink("SIG_TEST");  //删除进程中的信号量

	exit(0);
}


void pthreadStart(SerialPort *pemn)
{
    int sts = 0;
    pthread_attr_t         attr;                        //线程属性
    pthread_t readThread;
    pthread_t writeThread;
    pthread_t              thread_app_proess;           //应用业务线程
    pemn->threadExitFlag = 0;
    
    pthread_attr_init(&attr);                           //初始化线程属性
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); //设置线程属性

    // sts = pthread_create(&readThread, NULL, readProcess, NULL);
    // if (sts < 0)
    // {
    //     /* code */
    //     printf("can thread create failed\n");
    // }
    
    sts = pthread_create(&writeThread, NULL, writeProcess, NULL);
    if (sts < 0)
    {
        /* code */
        printf("serial thread create failed\n");
    }
    // return sts;
    // sts = pthread_create(&thread_app_proess, NULL, appProcess,NULL);
}



int main(int argc, char const *argv[])
{
    /* code */
    if (argv[2] == NULL)
    {
        printf("请输入串口名称");
        return -1;
    }
    // appHandle->path = argv[2];
    // appHandle->fd = 0;
    const char *path = argv[2];
    

    int sts = serialInit(fd, argv[1]);
    if (sts < 0)
    {
        printf("串口初始化失败\n");
        return -1;
    }
    else
    {
        printf("串口初始化成功\n");
    }


    //线程启动
    pthreadStart(&appHandle);
    if(sts < 0)
    {
        printf("pthread start failed\n");
    }
    
    signal(SIGKILL, die);
	signal(SIGINT, die);//当按下ctrl+c会执行die

	while (1)
	{
		sem_wait(g_signal);  //阻塞线程，避免空耗
        usleep(100*1000);	
	}

	die(0);

 

    return 0;
}