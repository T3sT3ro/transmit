// maksymilian_polarczyk
// 300791
#include "icmp_transmit.h"
#include "icmp_checksum.h"


void icmp_make_header(struct icmphdr* icmp_header, int pid, int seqNum) {
    icmp_header->type = ICMP_ECHO;
    icmp_header->code = 0;  // echo request
    icmp_header->un.echo.id = pid;
    icmp_header->un.echo.sequence = seqNum;
    icmp_header->checksum = 0;
    icmp_header->checksum = compute_icmp_checksum((u_int16_t*)icmp_header, sizeof(*icmp_header));
}

ssize_t packet_send(char* ip, int sockfd, int pid, int ttl, int seqNum) {
    // change ttl
    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

    // fillin address structure
    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));  // fill sizeof bytes with zeros
    recipient.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &recipient.sin_addr);

    // create data for packet
    struct icmphdr icmp_header;
    icmp_make_header(&icmp_header, pid, seqNum);

    // send packets through socket
    return sendto(sockfd, &icmp_header, sizeof(icmp_header), 0, (struct sockaddr*)&recipient, sizeof(recipient));
}

// sends 3 echo request packets with given ttl
int icmp_send_probe(int sockfd, char* ip, pid_t pid, int ttl){
    for (size_t i = 0; i < PACKETS_PER_GROUP; i++) {
            ssize_t bytes_sent = packet_send(ip, sockfd, pid, ttl, ttl * PACKETS_PER_GROUP + i);
            if (bytes_sent < 0)         
                return bytes_sent;
        }
    return 0;
}