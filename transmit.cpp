// maksymilian_polarczyk
// 300791
#include "common.h"

u_int8_t    window[MAX_WINDOW_SIZE][MAX_DATAGRAM_SIZE]; // window buffer
bool        window_received[MAX_WINDOW_SIZE];           // bool table of received datagrams
int         window_it = 0;                              // window as circular buffer
int         window_size = MAX_WINDOW_SIZE;              // size of current window
int         sockfd;                                     // socket file descriptor
int         outfd;                                      // output file descriptor
int         fsize;                                      // filesize from argument
struct sockaddr_in      server_addr;                    // server address struct
// suts up the output file and connection
void setup_environment(struct sockaddr_in &server_address, char **argv) {
    // open output file
    if ((outfd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC)) < 0) // override binary mode file
        criterr("cannot open output file");
    if ((fsize = strtoul(argv[4], NULL, 0)) <= 0 || errno) // fliesize
        criterr("invalid filesize");
    // ceil of fsize/MAX_DATAGRAM_SIZE
    window_size = min(MAX_WINDOW_SIZE, (fsize-1+MAX_DATAGRAM_SIZE)/MAX_DATAGRAM_SIZE);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (sockfd < 0)
        criterr("socket creation failed");

    // setup server_address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family       = AF_INET;
    server_address.sin_port         = htons(strtoul(argv[2], NULL, 10)); // port
    if (errno) criterr("port conversion failed");               // ERANGE from strtoul;
    if (!inet_pton(AF_INET, argv[1], &server_address.sin_addr)) // IP
        criterr("invalid IP address");
}

// requests part of file from server starting at offset with given size
void request_part(unsigned int offset, size_t size) {
    char msg[32];
    sprintf(msg, "GET %u %zu\n", offset, size);
    size_t msg_len = strlen(msg);
    if (sendto(sockfd, msg, msg_len, 0, 
               (struct sockaddr *)&server_addr, sizeof(server_addr))
        != (int) msg_len) 
        criterr("Sendto error");
}

int main(const int argc, char *argv[]) {
    // arguments: IP, port, filename, filesize
    if (argc != 5)
        criterr("Invalid arguments.\n Required format: IP port filename filesize.\n");

    setup_environment(server_addr, argv);

    struct timeval tv {0, DATAGRAM_TIMEOUT_MICROS};   // timeout to wait for reply

    while (window_size > 0){    // while still exists
        struct sockaddr_in      sender;
        socklen_t               sender_len = sizeof(sender);
        u_int8_t                buffer[IP_MAXPACKET + 1];       // datagram

        //request packets
        for (int i = 0; i < window_size; i++) // request missing packets
            if (!window_received[(window_it+i) % MAX_WINDOW_SIZE])
                request_part(
                    (window_it+i) * MAX_DATAGRAM_SIZE,
                    min(MAX_DATAGRAM_SIZE, fsize - (window_it + i) * MAX_DATAGRAM_SIZE)
                );

        // select with timeout for replies
        fd_set fds;     FD_ZERO(&fds);      FD_SET(sockfd, &fds);
        int ready = select(sockfd + 1, &fds, NULL, NULL, &tv);
        if (ready < 0)          criterr("selet ready<0");  // error
        else if (ready == 0)    continue;       // nothing came - request again

        ssize_t                 datagram_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0,
                                                        (struct sockaddr *)&sender, &sender_len);
        if (datagram_len < 0)   criterr("datagram length error");

        int     r_offset; // received data offset
        int     r_size;   // received data size

        if (sender.sin_port == server_addr.sin_port &&              // check server IP and PORT
            server_addr.sin_addr.s_addr == sender.sin_addr.s_addr &&
            sscanf((char*) buffer, "DATA %u %u\n", &r_offset, &r_size) == 2 &&    // check `DATA ...` header
            r_offset/MAX_DATAGRAM_SIZE >= window_it &&              // from current window
            r_offset/MAX_DATAGRAM_SIZE < window_it + window_size &&
            !window_received[(r_offset/MAX_DATAGRAM_SIZE) % MAX_WINDOW_SIZE])           // datagram not received
        {   // copy and mark datagram as read
            window_received[(r_offset/MAX_DATAGRAM_SIZE) % MAX_WINDOW_SIZE] = true;
            memcpy(window[(r_offset/MAX_DATAGRAM_SIZE) % MAX_WINDOW_SIZE], 
                   strchr((char *)buffer, '\n') + 1, r_size); 
        }

        // move window and copy data to file
        while(window_size > 0 && window_received[window_it % MAX_WINDOW_SIZE]){
            window_received[window_it % MAX_WINDOW_SIZE] = false;
            int data_len = min(MAX_DATAGRAM_SIZE, fsize - MAX_DATAGRAM_SIZE * window_it);
            if(write(outfd, window[window_it % MAX_WINDOW_SIZE], data_len) != data_len)
                criterr("data write error");
            ++window_it;
            if((window_it+window_size)*MAX_DATAGRAM_SIZE >= fsize)
                --window_size;
            printf("%.3f%% downloaded\n", 
            window_it*100.0 / (float)((fsize-1+MAX_DATAGRAM_SIZE)/MAX_DATAGRAM_SIZE));
            fflush(stdout);
        }

    }

    close(sockfd); // close the socket
    close(outfd); // close the output file
    return EXIT_SUCCESS;
}