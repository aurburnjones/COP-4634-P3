TARGET = lizard
CFLAGS = -g -Wall
LFLAGS = -lpthread
CC = gcc

$(TARGET) : $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LFLAGS)

clean :
	rm $(TARGET)
