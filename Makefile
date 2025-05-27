#!/bin/sh
#&& patchelf --replace-needed libzmq.so.5 libzmq.so ${BIN_TARGET}
#CC:=arm-linux-gnueabihf-gcc     arm-grammer-linux-gnueabi-gcc -mfloat-abi=hard -std=c11

#all:
	#$(CC) -o speed main.c can.c gpsdecode.c display.c pathplan.c navigationcontrol.c classdata.c com.c cubic_spline.c -lpthread -lm

# clean:
#	@rm -f speed
#*******************************************************

 DIR_INC  = ./inc
 DIR_SRC  = ./src
 DIR_OBJ  = ./obj
 DIR_BIN  = ./bin
 DIR_LIB  = ./lib
 DIR_NODE = ./node
  
 SRC = $(wildcard ${DIR_SRC}/*.c)  
 OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 
 
 TARGET = cantest
 
 BIN_TARGET = ${DIR_BIN}/${TARGET}
 LIBPATHS  = -L$(DIR_LIB)
 
 cp = a
 ifeq ($(cp),x86)
 	CC = arm-grammer-linux-gnueabi-gcc -mfloat-abi=hard
 else
	CC = arm-linux-gnueabihf-gcc
 endif
 CFLAGS = -Wall -I${DIR_INC}
 
 all:$(DIR_OBJ) $(BIN_TARGET)
 
 ${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -fPIC
 
 ifeq ($(cp),x86)
 ${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@ $(LIBPATHS) -lm -lpthread
 else 
 ${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@ $(LIBPATHS) -lm -lpthread
 endif
 
 .PHONY:clean
 clean:
	find ${DIR_OBJ} -name *.o -exec rm -rf {} \;
	find ${DIR_BIN} -name ${TARGET} -exec rm -rf {} \;
