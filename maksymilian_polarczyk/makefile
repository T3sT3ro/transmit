# maksymilian_polarczyk
# 300791
OBJS	= traceroute.o icmp_checksum.o icmp_transmit.o
SOURCE	= traceroute.cpp icmp_checksum.cpp icmp_transmit.cpp
HEADER	= icmp_checksum.h icmp_transmit.h common.h
OUT	= traceroute
CC	 = g++
FLAGS	 = -g -c -Wall -Wextra
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

traceroute.o: traceroute.cpp
	$(CC) $(FLAGS) traceroute.cpp -std=c++11

icmp_checksum.o: icmp_checksum.cpp
	$(CC) $(FLAGS) icmp_checksum.cpp -std=c++11

icmp_transmit.o: icmp_transmit.cpp
	$(CC) $(FLAGS) icmp_transmit.cpp -std=c++11

clean:
	rm -f $(OBJS) $(OUT)