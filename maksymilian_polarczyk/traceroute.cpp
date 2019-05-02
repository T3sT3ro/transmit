// maksymilian_polarczyk
// 300791
#include "common.h"
#include "icmp_checksum.h"
#include "icmp_transmit.h"

/// checks if string is a correct IPv4 format
bool isCorrectIPFormat(string ipString) {
    const regex formatRegex(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
    smatch match;
    if (regex_match(ipString, match, formatRegex)) {
        if (match.size() != 5 || match[0].compare(ipString) != 0) return false;
        for (size_t i = 1; i < match.size(); ++i) {
            string submatch = match[i].str();
            int num = stoi(submatch);
            if (num < 0 || num > 255) return false;
        }
    } else
        return false;
    return true;
}

int main(const int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    pid_t pid = getpid();  // pid will identify proper echo replies

    // check argument
    if (argc != 2 || !isCorrectIPFormat(string(argv[1]))) {
        fprintf(stderr, "Invalid argument. Pass correct IPv4 address as first argument\n");
        return EXIT_FAILURE;
    }

    // create socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // send packets with increasing ttl and measure time
    bool reached;
    for (int ttl = 1; !reached && ttl <= GROUP_CNT; ttl++) {
        // start calculating packet trips
        auto start = chrono::high_resolution_clock::now();

        // send 3 packets
        if (icmp_send_probe(sockfd, argv[1], pid, ttl) < 0) {
            fprintf(stderr, "packet send error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        set<string> ipstrings;
        long long avgTime = 0;

        size_t receivedProbeParts = 0;

        // timestamp for select
        fd_set descriptiors;
        FD_ZERO(&descriptiors);
        FD_SET(sockfd, &descriptiors);
        struct timeval tv {
            1, 0
        };

        // receive 3, drop bloat,
        while (receivedProbeParts < PACKETS_PER_GROUP) {
            // select with timeout of 1 sec
            int ready = select(sockfd + 1, &descriptiors, NULL, NULL, &tv);
            if (ready < 0) {
                fprintf(stderr, "socket error: %s\n", strerror(errno));
                return EXIT_FAILURE;
            } else if (ready == 0)
                break;

            // sender
            struct sockaddr_in sender;
            socklen_t sender_len = sizeof(sender);
            // packet
            u_int8_t buffer[IP_MAXPACKET];
            ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
            if (packet_len < 0) {
                fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }

            // ip header
            struct iphdr* ip_header = (struct iphdr*)buffer;

            // icmp header
            u_int8_t* icmp_packet = buffer + 4 * ip_header->ihl;
            struct icmphdr* icmp_header = (struct icmphdr*)icmp_packet;

            // received icmp header for TTL-exceeded
            struct iphdr* rec_ip_header = (struct iphdr*)((u_int8_t*)icmp_header + 8);
            u_int8_t* rec_icmp_packet = ((u_int8_t*)rec_ip_header) + 4 * rec_ip_header->ihl;
            struct icmphdr* rec_icmp_header = (struct icmphdr*)rec_icmp_packet;

            if ((icmp_header->type == ICMP_ECHOREPLY && icmp_header->un.echo.id == pid &&
                 icmp_header->un.echo.sequence / PACKETS_PER_GROUP == ttl) ||
                (icmp_header->type == ICMP_TIME_EXCEEDED && rec_icmp_header->un.echo.id == pid &&
                 rec_icmp_header->un.echo.sequence / PACKETS_PER_GROUP == ttl)) {
                // add to addresses that responded
                auto elapsed = std::chrono::high_resolution_clock::now() - start;
                avgTime += std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

                char ip_str[20];
                inet_ntop(AF_INET, &sender.sin_addr, ip_str, sizeof(ip_str));
                ipstrings.insert(string(ip_str));

                receivedProbeParts++;
            }
            reached = icmp_header->type == ICMP_ECHOREPLY;
        }

        printf("%2d.\t", ttl);
        if (receivedProbeParts > 0) {
            // many routers gave reply
            for (auto ipstr : ipstrings) printf("%s\t", ipstr.c_str());
            if (receivedProbeParts == PACKETS_PER_GROUP)
                printf("[%.3fms]\n", avgTime / 3000.0);
            else  // packet loss occured
                printf("[???]\n");
        } else  // router didn't give answer
            printf("*\n");
    }
    return EXIT_SUCCESS;
}