# Makefile

CC := gcc
ASM := nasm
CFLAGS := -Wall -g
ASMFLAGS := -f elf64 -g

TARGET := myprog
OBJS := main.o context_swap.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

context_swap.o: context_swap.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
