# maksymilian_polarczyk
# 300791
OBJS	= transmit.o
SOURCE	= transmit.cpp
HEADER	= transmit.h common.h
OUT	= transmit
CC	 = g++
FLAGS	 = -g -c -Wall -Wextra
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

transmit.o: transmit.cpp
	$(CC) $(FLAGS) transmit.cpp -std=c++11

clean:
	rm -f $(OBJS) $(OUT)
