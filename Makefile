# Makefile

CC := gcc
ASM := nasm
CFLAGS := -Wall -g
ASMFLAGS := -f elf64 -g

TARGET := myprog
OBJS := main.o stack_switcher.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

stack_switcher.o: stack_switcher.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
