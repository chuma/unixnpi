CC = gcc

TARGET = unixnpi

SRCS = newtmnp.c unixnpi.c

all: unixnpi

debug:
	cp *.c backup
	cp *.h backup
	$(CC) -g -o $(TARGET) $(SRCS)
	
unixnpi:
	$(CC) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

