#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>


struct __attribute__((packed)) dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

struct dns_question {
    uint8_t* qname;
    uint16_t qtype;
    uint16_t qclass;
} __attribute__((packed));

struct dns_answer {
    uint8_t* name;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t* rdata; // Same length as rdlength
} __attribute__((packed));

#define QR_FLAG (1 << 15);
#define RD_BIT (1 << 8)
int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    int ret = inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);
    if (ret < 0) {
        perror("inet_pton failed");
        return -1;
    }

    ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        perror("Connect failed");
        return -1;
    }

    struct dns_header dns_hdr;
    memset(&dns_hdr, 0, sizeof(dns_hdr));
    dns_hdr.id = htons(1337);

    dns_hdr.flags &= ~QR_FLAG;
    dns_hdr.flags |= RD_BIT;

    dns_hdr.qdcount = 1;
    dns_hdr.ancount = 0;
    dns_hdr.nscount = 0;
    dns_hdr.arcount = 0;

    struct dns_question dns_q;
    memset(&dns_q, 0, sizeof(dns_q));


    close(sock);

    // --------

    // struct addrinfo hints;
    // memset(&hints, 0, sizeof(hints));
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_protocol = 0;

    // struct addrinfo* result;
    // int ret = getaddrinfo("google.com", "443", &hints, &result);
    // if (ret < 0) {
    //     perror("Get addr info failed");
    //     return -1;
    // }

    // struct addrinfo* ptr = NULL;
    // for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
    //     switch (ptr->ai_family) {
    //         case AF_INET: {
    //             struct sockaddr_in* addr = (struct sockaddr_in*)ptr->ai_addr;
    //             char buf[INET_ADDRSTRLEN];
    //             const char* r = inet_ntop(addr->sin_family, &addr->sin_addr, buf, sizeof(buf));
    //             if (!r) {
    //                 fprintf(stderr, "inet_ntop failed v4\n");
    //             }

    //             printf("Addresss: %s, port: %d\n\n", buf, ntohs(addr->sin_port));
    //             break;
    //         }
    //         case AF_INET6: {
    //             struct sockaddr_in6* addr = (struct sockaddr_in6*)ptr->ai_addr;
    //             char buf[INET6_ADDRSTRLEN];
    //             const char* r = inet_ntop(addr->sin6_family, &addr->sin6_addr, buf, sizeof(buf));
    //             if (!r) {
    //                 fprintf(stderr, "inet ntp failed v6\n");
    //             }
    //             printf("Ipv6 addr: %s\n", buf);
    //             break;
    //         }
    //         default: {
    //             fprintf(stderr, "not supported\n");
    //             break;
    //         }
    //     }
    // }

    // freeaddrinfo(result);
    return 0;
}
