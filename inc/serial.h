#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <pthread.h>

typedef struct
{
    int fd;
    // int baudrate;
    // int databits;
    pthread_t readThreadID;
    pthread_t writeThreadID;

    const char *path;
    int threadExitFlag;


}SerialPort;



int serialInit(int fd, const char *device);

int appProcess(int fd);

int appParamProcess(int fd , int id, const char *path);
void printfParam();

int sendParam(int fd, const char *cmd, const char **params, int param_count);
int sendNoParam(int fd, const char *cmd);

void readProcess(SerialPort *arg);
void writeProcess(SerialPort *arg);
void pthreadStop(SerialPort *pemn);
void pthreadStart(SerialPort *pemn);
#endif
