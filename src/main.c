#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"

int serial_id = 0;
int main(int argc, char const *argv[])
{
    /* code */
    
    int sts = serialInit(serial_id);
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

    while (1)
    {

        printf("请输入读取参数:\n");
        
        scanf("%d", &param_id);

        // appProcess(serial_id);
        appParamProcess(sts , param_id);
        usleep(100*1000);	
        /* code */
    }
    
    

    return 0;
}