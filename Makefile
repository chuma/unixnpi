# change this if your compiler isn't GCC
CC = cc

TARGET = unixnpi

SRCS = newtmnp.c unixnpi.c

all: unixnpi

debug:
	cp *.c backup
	cp *.h backup
	$(CC) -g -o $(TARGET) $(SRCS)
	
unixnpi: $(SRCS)
	$(CC) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

