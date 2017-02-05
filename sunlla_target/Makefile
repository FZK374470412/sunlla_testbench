TARGET = sunlla_testbench

DIR_CODE = ./core ./xml ./test
DIR_INC = -I./core -I./xml -I./test -I./include -I.

SRC = $(foreach dir, $(DIR_CODE), $(wildcard $(dir)/*.c))
OBJ = $(patsubst %.c, %.o, $(SRC))

CC = /home/vedic/pro/pi3/tools/raspbian-toolchain-gcc-4.7.2-linux64/bin/arm-linux-gnueabihf-gcc

LDFLAGS = -lpthread -lrt
CFLAGS = $(DIR_INC)

all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ):%.o:%.c
	@echo Compiling $@ ...
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET)
	@rm -rf $(OBJ)

.PHONY: all clean
