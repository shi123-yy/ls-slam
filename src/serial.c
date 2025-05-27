#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <semaphore.h>
#include "serial.h"
#define ERROR -1

sem_t  *g_signal = NULL; //调试线程信号量


// //关闭设备
// void die(int x)
// {
// 	// freeSpi();
// 	// pthreadStop(&appHandle);

// 	sem_close(g_signal);     //关闭信号量
// 	sem_unlink("SIG_TEST");  //删除进程中的信号量

// 	exit(0);
// }
void serialInit(int fd)
{
    
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        printf("open serial failed\n");
    }
    struct termios options;

    tcgetattr(fd, &options);

    options.c_cflag |= (CLOCAL | CREAD);                    // 忽略调制解调器状态线，启用接收器
    options.c_cflag &= ~PARENB;                             // 无奇偶校验
    options.c_cflag &= ~CSTOPB;                             // 1位停止位
    options.c_cflag &= ~CSIZE;                              // 清除数据位掩码
    options.c_cflag |= CS8;                                 // 8位数据位
    options.c_cflag |= B115200;                             // 设置波特率115200
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);     // 禁用规范模式（原始输入）
    options.c_oflag &= ~OPOST;                              // 禁用输出处理（原始输出）

    tcflush(fd, TCIOFLUSH);                          // 清空缓冲区
    tcsetattr(fd, TCSANOW, &options);                // 立即应用配置
    
    return fd;
}

int appProcess(int fd)
{

    FILE *file = fopen("./config/config.txt", "r+");
    if(file == NULL)
    {
        printf("Error opening file\n");
        return ERROR;
    }

    char buffer[256] = {0};
    while(fgets(buffer, sizeof(buffer), file) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';

        char *token = strtok(buffer, ",");
        if(token == NULL)
        {
            continue;
        }

        if(0 == strcmp(token, "SETNET")) //设置网络参数
        {
            const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if(0 == strcmp(token, "GETNET"))//获取网络参数
        {
            sendNoParam(fd, token);
        }
        else if(0 == strcmp(token, "NETSTATE"))//上传网络状态
        {
            const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if (0 == strcmp(token , "SETAPN"))//设置APN参数
        {
            const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if(0 == strcmp(token, "GETAPN"))//获取APN参数
        {
            sendNoParam(fd, token);
            
        }
        else if (0 == strcmp(token, "GETICCID"))//获取流量卡参数
        {
            /* code */
            sendNoParam(fd, token);
        }
        else if (0 == strcmp(token, "CONT_CRC_ERROR"))//接收命令时CRC校验码错误
        {
            /* code */
            sendNoParam(fd, token);
        }
        else if (0 == strcmp(token, "GET_PN_INFO"))//查询控制器PN号
        {
            /* code */
            sendNoParam(fd, token);
        }
        else if (0 == strcmp(token, "CLEAN_EPH"))//清除星历
        {
            sendNoParam(fd, token);
        }
        else if (0 == strcmp(token, "LOCATION"))//关闭支持的卫星
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if (0 == strcmp(token, "GET_DEVICE_MSG"))//获取设备信息
        {
           sendNoParam(fd, token);
        }
        else if (0 == strcmp(token, "SET_WORKMODE"))//设置工作模式
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if (0 == strcmp(token, "SET_CONFIG"))//设置电台参数
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if (0 == strcmp(token, "SET_TRACKER_INSTALL_PARAM"))//设置拖拉机安装参数
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if (0 == strcmp(token, "SET_ANGLE_INSTALL_PARAM"))//设置姿态角安装误差参数
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if (0 == strcmp(token, "HCNX,SET_SINGLE_BEIDOU_MODE"))//设置单北斗模式
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if (0 == strcmp(token, "SET_HEADING_INVERSION"))//设置航向取反
        {
           sendNoParam(fd, token);
        }
        else if (0 == strcmp(token, "SET_VEH_CTRLPOINT_OUTPUT_PARAM"))//设置是否打开车辆控制点坐标输出即车辆类型
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }
        else if(0 == strcmp(token,"GET_VEH_CTRLPOINT_OUTPUT_PARAM"))//获取是否打开车辆控制点坐标输出即车辆类型
        {
            sendNoParam(fd, token);
        }
        else if (0 == strcmp(token, "SET_VEH_WHEEL_BASE"))//设置车辆前后轴距离
        {
           const char *param = strtok(NULL, ",");
            if(param == NULL)
            {
                printf("Invalid SETNET command\n");
                continue;
            }
            sendParam(fd , token, param);
        }

        



    }
    return 0;
    
}


void printfParam()
{
    printf("设置网络参数[1]\n");
    printf("获取网络参数[2]\n");
    printf("上传网络状态[3]\n");
    printf("设置APN参数[4]\n");
    printf("获取APN参数[5]\n");
    printf("获取流量卡参数[6]\n");
    printf("接收命令时CRC校验码错误[7]\n");
    printf("查询控制器PN号[8]\n");
    printf("清除星历[9]\n");
    printf("关闭支持的卫星[10]\n");
    printf("获取设备信息[11]\n");
    printf("设置工作模式[12]\n");
    printf("设置电台参数[13]\n");
    printf("设置拖拉机安装参数[14]\n");
    printf("设置姿态角安装误差参数[15]\n");
    printf("设置单北斗模式[16]\n");
    printf("设置航向取反[17]\n");
    printf("设置是否打开车辆控制点坐标输出即车辆类型[18]\n");
    printf("获取是否打开车辆控制点坐标输出即车辆类型[19]\n");
    printf("设置车辆前后轴距离[20]\n");

}

//计算检校码
unsigned char bcc_calc(const char *data, size_t len) 
{
    unsigned char xor = 0x00;
    for (size_t i = 0; i < len; i++) 
    {
        xor ^= (unsigned char)data[i];
    }
    return xor;
}


int sendNoParam(int fd, const char *cmd)
{
    char full_cmd[256] = {0};
    char *ptr = full_cmd;

    ptr += sprintf(ptr, "$HCNX,%s", cmd);

    const char *bcc = strchr(full_cmd, '$')+1;
    size_t bcc_len = ptr - bcc;
    unsigned char calculated_bcc = bcc_calc(bcc, bcc_len);
    ptr += sprintf(ptr, "*%02X\r\n", calculated_bcc);
    
    int len = write(fd, full_cmd, strlen(full_cmd));
    if(len < 0)
    {
        printf("write failed\n");
        return ERROR;
    }

    printf("send param: %s\n", full_cmd);
}

int sendParam(int fd, const char *cmd, const char *param)
{
    char full_cmd[256] = {0};
    char *ptr = full_cmd;

    ptr += sprintf(ptr, "$HCNX,%s", cmd);
    for(int i = 0; i < strlen(param); i++)
    {
        ptr += sprintf(ptr, "%c", param[i]);
    }

    const char *bcc = strchr(full_cmd, '$')+1;
    size_t bcc_len = ptr - bcc;
    unsigned char calculated_bcc = bcc_calc(bcc, bcc_len);
    ptr += sprintf(ptr, "*%02X\r\n", calculated_bcc);
    
    int len = write(fd, full_cmd, strlen(full_cmd));
    if(len < 0)
    {
        printf("write failed\n");
        return ERROR;
    }

    printf("send param: %s\n", full_cmd);
}

