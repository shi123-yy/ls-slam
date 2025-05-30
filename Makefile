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
 
 TARGET = paramtest
 
 BIN_TARGET = ${DIR_BIN}/${TARGET}
 LIBPATHS  = -L$(DIR_LIB)
 
 cp = arm;
 ifeq ($(cp),x86)
 	CC = arm-grammer-linux-gnueabi-gcc -mfloat-abi=hard
 else
	CC = arm-linux-gnueabihf-gcc
 endif

 CFLAGS = -Wall -O2 -I${DIR_INC}
 LDFLAGS = -static
 all:$(DIR_OBJ) $(BIN_TARGET)
 
 ${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -fPIC
 
 ifeq ($(cp),x86)
 ${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@ $(LIBPATHS) -lm
 else 
 ${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@ $(LIBPATHS) -lm
 endif
 
 verify:
	@echo "验证静态编译:"
	file ${DIR_BIN}/${TARGET}
	ldd ${DIR_BIN}/${TARGET} || echo "静态链接成功 - 不是动态可执行文件"
	size ${DIR_BIN}/${TARGET}


 .PHONY:clean
 clean:
	find ${DIR_OBJ} -name *.o -exec rm -rf {} \;
	find ${DIR_BIN} -name ${TARGET} -exec rm -rf {} \;


