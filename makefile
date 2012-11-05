-include ./path.mak

CC := gcc #$(LINUXarm_GCC)
CFLAGS := -g
LINKER_FLAGS :=
TARGET := adafruitRotaryEncoder.arm

.PHONY : all

all : $(TARGET)
	$(CC) $(CFLAGS) *.o -o $(TARGET)

$(TARGET) : *.o

*.o : *.c
	$(CC) $(CFLAGS) -c *.c

clean :
	rm -f *.o $(TARGET)
