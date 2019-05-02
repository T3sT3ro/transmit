#ifndef TRANSMIT_COMMON_H
#define TRANSMIT_COMMON_H

using namespace std;

#define WINDOW_SIZE 10
#define MAX_DATAGRAM_SIZE 4096

#define GROUP_CNT 30
#define PACKETS_PER_GROUP 3
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <errno.h>
#include <iostream>
#include <netinet/ip_udp.h>
#include <netinet/ip.h>
#include <regex>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h> // getpid


#endif //TRANSMIT_COMMON_H