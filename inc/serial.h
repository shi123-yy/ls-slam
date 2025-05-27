#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>


void serialInit(int fd);

int appProcess(int fd);

void printfParam();

int sendParam(int fd, const char *cmd, const char *param);
int sendNoParam(int fd, const char *cmd);

#endif



