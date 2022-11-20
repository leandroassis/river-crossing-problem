CC = gcc

CFLAGS  = -Wall -Werror -D_POSIX_THREAD_SEMANTICS
LDFLAGS = -pthread

TARGET = river_crossing

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS)

clean:
	$(RM) $(TARGET)
