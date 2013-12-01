#Nate Gillard
#cs457
#LinkStateRoutingSim: Project 3 Simulatiing a Link State Routing Protocol.
CC = g++
CFLAGS = -Wall -Wextra -ansi -gstabs
OBJS = manager.o

all: $(OBJS)
	$(CC) $(CFLAGS) manager.o -o manager

%.o: %.cc
	$(CC) $(CFLAGS) -c $<

clean:
	rm -v manager *.o *.out
