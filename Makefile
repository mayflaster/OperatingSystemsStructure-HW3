# Makefile for the Bank program
CC = gcc
CFLAGS = -g -Wall
CCLINK = $(CC)
OBJS = main.o Packet.o
RM = rm -f

# Creating the  executable
all: ttftps
ttftps: $(OBJS)
	$(CCLINK) $(CFLAGS) -o ttftps $(OBJS)

# Creating the object files
Packet.o: Packet.c Packet.h 
main.o: main.c Packet.h 

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*
