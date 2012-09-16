TARGET=appli_FM.elf
OBJECTS=appli_FM.o
CC=avr32-linux-gcc
CFLAGS=-Wall -g # warnings, debugging symbols
LDFLAGS=-lpthread -lrt
LIBS=

.PHONY: all
all: $(TARGET) 

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) 

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS) 

