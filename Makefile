CC = gcc

TARGET = unixnpi

SRCS = newtmnp.c encrypt.c unixnpi.c

all: elf

debug:
	cp *.c backup
	cp *.h backup
	$(CC) -g -m486 -o $(TARGET) $(SRCS)
	$(CC) -g -m486 -o regcode regcode.c encrypt.c newtmnp.c
	
elf:
	$(CC) -o $(TARGET) $(SRCS)

aout:
	$(CC) -b i486-linuxaout -o $(TARGET) $(SRCS)
