CC = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -static
TARGET = av_simulator

all: $(TARGET)

$(TARGET): av_simulator.c
	$(CC) $(CFLAGS) -o $(TARGET) av_simulator.c

clean:
	rm -f $(TARGET)

