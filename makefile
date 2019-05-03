# maksymilian_polarczyk
# 300791
OBJS	= transport.o
SOURCE	= transport.cpp
HEADER	= transport.h common.h
OUT	= transport
CC	 = g++
FLAGS	 = -g -c -Wall -Wextra
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

transport.o: transport.cpp
	$(CC) $(FLAGS) transport.cpp -std=c++11

clean:
	rm -f $(OBJS)

distclean:
	rm -f $(OUT)
