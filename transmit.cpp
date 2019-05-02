// maksymilian_polarczyk
// 300791
#include "common.h"

u_int8_t    window[MAX_WINDOW_SIZE][MAX_DATAGRAM_SIZE]; // window buffer
bool        window_received[MAX_WINDOW_SIZE];           // bool table of received datagrams
int         window_it = 0;                              // window as circular buffer
int         window_size = MAX_WINDOW_SIZE;              // size of current window
int         sockfd;                                     // socket file descriptor
FILE *      outfd = NULL;                               // output file descriptor
int         fsize;                                      // filesize from argument

// suts up the output file and connection
void setup_environment(struct sockaddr_in &server_address, char **argv) {
    // open output file
    if (!fopen(argv[3], "wb")) // override binary mode file
        criterr("Cannot open output file.\n");
    if ((fsize = stroul(argv[4])) || errno) // fliesize
        criterr("Invalid filesize.\n");
    // ceil of fsize/MAX_DATAGRAM_SIZE
    window_size = min(MAX_WINDOW_SIZE, (fsize-1+MAX_DATAGRAM_SIZE)/MAX_DATAGRAM_SIZE);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (sockfd < 0)
        criterr(NULL);

    // setup server_address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family   = AF_INET;
    server_address.sin_port     = strtoul(argv[2], NULL, 10); // port
    if (errno) // check if port is 
        criterr(); // ERANGE from strtoul;

    // check IP format
    if (!inet_pton(AF_INET, argv[1], &server_address.sin_addr.s_addr)) // IP
        criterr("Invalid IP address.\n");

    // convert host long-> network long
    server_address.sing_port        = htons(server_address.sing_port);
    server_address.sin_addr.s_addr  = htonl(server_address.sin_addr.s_addr));


    if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        criterr();
}

// requests part of file from server starting at offset with given size
void request_part(unsigned int offset, size_t size) {
    char msg[32];
    sprintf(msg, "GET %u %u\n", offset, size);
    size_t msg_len = strlen(msg);
    if (sendto(sockfd, msg, msg_len 0, 
               (struct sockaddr *)&sender, &sender_len)
        != msg_len) 
        criterr();
}

int main(const int argc, char *argv[]) {
    // arguments: IP, port, filename, filesize
    if (argc != 5)
        criterr("Invalid arguments. Required: IP port filename filesize.\n");

    struct sockaddr_in server_addr;
    setup_environment(server_addr, argv);

    struct timeval tv {0, DATAGRAM_TIMEOUT_MICROS};   // timeout to wait for reply

    while (window_size > 0){    // while still exists
        //request packets
        for (int i = window_it; i < window_size; i++) // request missing packets
            if (!window_received[window_it % MAX_WINDOW_SIZE])
                request_part(
                    window_it * MAX_DATAGRAM_SIZE,
                    min(MAX_DATAGRAM_SIZE, fsize - MAX_DATAGRAM_SIZE * window_it)
                );

        // select with timeout of 0.01 sec for replies
        fd_set fds;     FD_ZERO(&fds);      FD_SET(sockfd, &fds);
        int ready = select(sockfd + 1, &fds, NULL, NULL, &tv);
        if (ready < 0)          criterr(NULL);  // error
        else if (ready == 0)    continue;       // nothing came - request again

        struct sockaddr_in      sender;
        socklen_t               sender_len = sizeof(sender);
        u_int8_t                buffer[IP_MAXPACKET + 1]; // packet
        ssize_t                 datagram_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0,
                                                        (struct sockaddr *)&sender, &sender_len);
        if (datagram_len < 0)   criterr(NULL);

        int     r_offset; // received data offset
        int     r_size;   // received data size

        if (sender.sin_port == server_addr.sin_port &&              // check server IP and PORT
            server_addr.sin_addr.s_addr == sender.sin_addr.s_addr &&
            sscanf("DATA %zu %zu\n", &r_offset, &r_size) == 2 &&    // check `DATA ...` header
            r_offset/MAX_DATAGRAM_SIZE >= window_it &&              // from current window
            r_offset/MAX_DATAGRAM_SIZE < window_it + window_size &&
            !window_received[r_offset/MAX_DATAGRAM_SIZE])           // datagram not received
        {   // copy and mark datagram as read
            window_received[(r_offset/MAX_DATAGRAM_SIZE) % MAX_WINDOW_SIZE] = true;
            memcpy(window[r_offset % MAX_WINDOW_SIZE], strchr(buffer, '\n') + 1, r_size); 
        }

        // move window and copy data to file
        while(window_received[window_it % MAX_WINDOW_SIZE]){
            window_received[window_it % MAX_WINDOW_SIZE] = false;
            write(outfd, window[window_it % MAX_WINDOW_SIZE], // append data to the file
                  min(MAX_DATAGRAM_SIZE, fsize - MAX_DATAGRAM_SIZE * window_it));
            ++window_it;
            if((window_it+window_size)*MAX_DATAGRAM_SIZE >= fsize)
                --window_size;
            printf("%.3f%% downloaded\n", (float)window_it / (float)fsize);
        }

    }

    close(sockfd); // close the socket
    close(outfd);  // close the output file
    return EXIT_SUCCESS;
}