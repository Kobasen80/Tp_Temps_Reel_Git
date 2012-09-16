TARGET=appli_Multithread.elf
OBJECTS=appli_Multithread.o
CC=avr32-linux-gcc
CFLAGS=-Wall -g # warnings, debugging symbols
LDFLAGS=-lpthread
LIBS=

.PHONY: all
all: $(TARGET) 

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) 

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS) 

