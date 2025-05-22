# 编译器
# CXX = g++
CXX = gcc
# 获取当前目录
CURRENT_DIR := $(CURDIR)
# 编译选项
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# 目标可执行文件名
# TARGET = ls_serial
# TARGET = gpsToUTM
TARGET = slam
# TARGET = utm2gps
# 源文件
SRCS = src/slam.c
# SRCS = src/gptoutm.c

# 头文件路径
INCLUDES = -I/opt/ros/noetic/include

# 库路径
LDFLAGS = -L/opt/ros/noetic/lib
# LDFLAGS = lib
# 库目录设置为当前目录下的 lib 文件夹
# DIR_LIB  = $(CURRENT_DIR)/lib
# 链接库
# LDLIBS = -lserial
# LDLIBS = -lproj
LDLIBS = -lm -lproj 

# BIN_TARGET = ${DIR_BIN}/${TARGET}
# 默认目标
all: $(TARGET)

# 生成可执行文件
${TARGET}: $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

# 清理生成的文件
clean:
	rm -f $(TARGET) *.o

# 重新编译
rebuild: clean all

# 伪目标
.PHONY: all clean rebuild