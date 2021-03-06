// maksymilian_polarczyk
// 300791
#ifndef TRANSMIT_COMMON_H
#define TRANSMIT_COMMON_H

using namespace std;

#define MAX_WINDOW_SIZE 1000               // number of datagrams per window
#define MAX_DATAGRAM_SIZE 1000        // maximum single data in datagram
#define DATAGRAM_TIMEOUT_MICROS 10000 // 0.01 second timeout to resend GET request
#define MAX_FILESIZE 10001000         // maximum filesize to send

#include <arpa/inet.h>
//#include <Winsock2.h>

#include <chrono>
#include <type_traits>
#include <ctime>
#include <errno.h>
#include <iostream>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <regex>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> // O_FLAGS

typedef result_of<decltype(chrono::high_resolution_clock::now)&(void)>::type chronoret;
void criterr(const char *str) // error handling function
{
    fprintf(
        stderr,
        "error: %s errno: %s\n", str, strerror(errno));
    exit(EXIT_FAILURE);
}

#endif //TRANSMIT_COMMON_H