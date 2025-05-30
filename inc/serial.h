#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>


int serialInit(int fd, const char *device);

int appProcess(int fd);

int appParamProcess(int fd , int id, const char *path);
void printfParam();

int sendParam(int fd, const char *cmd, const char **params, int param_count);
int sendNoParam(int fd, const char *cmd);

#endif



