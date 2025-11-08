#include <stdint.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include "dns.h"

// REosurces:
// https://courses.cs.duke.edu/fall16/compsci356/DNS/DNS-primer.pdf
// https://www.geeksforgeeks.org/computer-networks/dns-message-format/

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stdout, "Expected to get <[v4 || v6]> <dns-server-ip> <domain-name>\n");
        return 0;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    int ret = inet_pton(AF_INET, argv[2], &addr.sin_addr); // Google dns
    if (ret < 0) {
        perror("inet_pton failed");
        return -1;
    }

    ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr)); // since now i can use send instead of sendto
    if (ret < 0) {
        perror("Connect failed");
        return -1;
    }

    int qtype;
    if (strcmp("v4", argv[1]) == 0) {
        qtype = IPV4_RECORD;
    } else if (strcmp("v6", argv[1]) == 0) {
        qtype = IPV6_RECORD;
    } else {
        fprintf(stderr, "Unknown type\n");
        return -1;
    }
    ret = send_dns_request(sock, qtype, argv[3]);
    if (ret < 0) {
        fprintf(stderr, "Send dns request failed\n");
        return -1;
    }

    ret = parse_dns_response(sock);
    if (ret < 0) {
        fprintf(stderr, "Parse dns response failed\n");
        return -1;
    }

    close(sock);
    return 0;
}
