# 定义编译器和工具
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

# 如果没有定义 BUILD_DIR，则使用当前目录
BUILD_DIR ?= .

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0
    LDFLAGS += -g
endif

# 获取所有 .c 文件和对应的 .o 文件
clibs := $(wildcard *.c)
objs := $(clibs:%.c=$(BUILD_DIR)/%.o)

# 清理目标
.PHONY: clear clean distclean distclear fileclean all

all: $(BUILD_DIR)/libMyLib.a

# 创建构建目录(如果需要)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 默认目标：生成静态库 MyLib.a
$(BUILD_DIR)/libMyLib.a: $(objs)
	$(AR) rcs $@ $^

# 规则：从 .c 文件生成 .o 文件
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


clear clean distclean distclear:
	-rm -rf $(BUILD_DIR)/*.o 
	-rm -rf $(BUILD_DIR)/libMyLib.a

fileclean:
	-rm -rf *.c1
	-rm -rf *.h1
	-rm -rf *.c
	-rm -rf *.h
