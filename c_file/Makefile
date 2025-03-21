# 设置编译器
CC = gcc

# 编译选项：
CFLAGS = -Wall -g -pthread

# 包含头文件目录（-I 链接 头文件 include 路径）
INCLUDES = -I./include

# 源文件列表（主程序和子模块）
SRCS = main.c src/callback.c

# 目标程序名
TARGET = main

# 默认目标（make 时执行）
all: $(TARGET)

# 编译目标程序
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)

# 清理目标文件和可执行文件
clean:
	rm -f $(TARGET)

