#include <stdint.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include "dns.h"

#define GOOGLE_DNS "8.8.8.8"

#define QR_FLAG (1 << 15)
#define AA_BIT (1 << 10)
#define TRUNC_BIT (1 << 9)
#define RD_BIT (1 << 8)
#define RA_BIT (1 << 7)
#define RCODE_MASK (0b0000000000001111)
#define MAX_DOMAIN_LENGTH 255 // 253 + padding j in case
#define MAX_DNS_LENGTH 512
// REosurces:
// https://courses.cs.duke.edu/fall16/compsci356/DNS/DNS-primer.pdf
// https://www.geeksforgeeks.org/computer-networks/dns-message-format/


char* process_domain(const char* domain) {
    if (!domain) return NULL;

    char* result_buf = malloc(MAX_DOMAIN_LENGTH);
    int result_offset = 0;

    const char* base = domain;
    char* found;
    while ((found = strchr(base, '.')) != NULL) {
        int len = found - base;

        result_buf[result_offset] = len;
        ++result_offset;

        memcpy(result_buf + result_offset, base, len);
        result_offset += len;

        base = ++found;
    }
    if (result_offset == 0) { // Didn't parse anything in while
        free(result_buf);
        return NULL;
    }

    int len = domain + strlen(domain) - base;

    result_buf[result_offset] = len;
    ++result_offset;

    memcpy(result_buf + result_offset, base, len);
    result_offset += len;
    result_buf[result_offset] = '\0';
    ++result_offset;

    result_buf = realloc(result_buf, result_offset);
    printf("dom: %s, len: %d\n", result_buf, strlen(result_buf));

    return result_buf;
}

int send_dns_request(int sock, const char* domain) {
    int ret = 0;
    char buf[MAX_DNS_LENGTH];
    char* buf_ptr = buf;
    struct dns_header* dns_hdr = (struct dns_header*)buf_ptr;
    memset(dns_hdr, 0, sizeof(*dns_hdr));
    dns_hdr->id = htons(1337);
    dns_hdr->flags = htons(ntohs(dns_hdr->flags) & ~QR_FLAG);
    dns_hdr->flags = htons(ntohs(dns_hdr->flags | RD_BIT));
    dns_hdr->qdcount = htons(1);
    dns_hdr->ancount = htons(0);
    dns_hdr->nscount = htons(0);
    dns_hdr->arcount = htons(0);
    buf_ptr += sizeof(*dns_hdr);

    char* qname_bytes = process_domain(domain);
    if (!qname_bytes) {
        fprintf(stderr, "Process domain failed\n");
        return -1;
    }
    size_t qname_len = strlen(qname_bytes) + 1;
    memcpy(buf_ptr, qname_bytes, qname_len);
    buf_ptr += qname_len;

    struct dns_question* dns_q = (struct dns_question*)(buf_ptr);
    memset(dns_q, 0, sizeof(*dns_q));

    dns_q->qtype = htons(0x0001);
    dns_q->qclass = htons(0x0001);
    buf_ptr += sizeof(*dns_q);

    ret = send(sock, buf, buf_ptr - buf, 0);
    if (ret < 0) {
        perror("send failed");
        return -1;
    }

    free(qname_bytes);
    return 0;
}

int parse_dns_response(int sock) {
    char dns_buf[MAX_DNS_LENGTH]; // 512 - max guaranteed bytes size
    ssize_t rd_bytes = read(sock, dns_buf, sizeof(dns_buf));
    if (rd_bytes < 0) {
        perror("Read error");
        return -1;
    }

    // HEader paraing
    char* dns_buf_p = dns_buf;
    struct dns_header* dns_hdr = (struct dns_header*)dns_buf_p;
    dns_buf_p += sizeof(struct dns_header);

    uint16_t flags = ntohs(dns_hdr->flags);
    bool is_ra = (flags & RA_BIT);
    bool is_rcode = (flags & RCODE_MASK);
    if (!is_ra) {
        fprintf(stderr, "Recursion is not Available\n");
        return -1;
    }
    if (is_rcode != 0) {
        fprintf(stderr, "DNS request contains an error, %d\n", (flags & RCODE_MASK));
        return -1;
    }

    printf("Questions: %d, Ansers: %d\nRecords: %d, ARecords: %d\n",
        ntohs(dns_hdr->qdcount), ntohs(dns_hdr->ancount), ntohs(dns_hdr->nscount), ntohs(dns_hdr->arcount));

    // Question parsing
    char* qname = dns_buf_p;
    if ((*qname & 0xC0) == 0xC0) {
        perror("Pointer, skip for now");
        return -2;
    }
    printf("Str: %s\n", qname);
    dns_buf_p += strlen(qname) + 1; // skip string + null terminator

    uint16_t qtype = ntohs(*((uint16_t*)dns_buf_p));
    dns_buf_p += sizeof(uint16_t);
    uint16_t qclass = ntohs(*((uint16_t*)dns_buf_p));
    dns_buf_p += sizeof(uint16_t);
    printf("Qtype: %d, qclass: %d\n", qtype, qclass);

    // Question parsing

    char* name = dns_buf_p;
    if ((*name & 0xC0) == 0xC0) {
        perror("ptr in answer skip for now");
        return -1;
    }
    printf("Name: %s\n", name);
    dns_buf_p += strlen(name) + 1;

    uint16_t type = ntohs(*(uint16_t*)dns_buf_p);
    dns_buf_p += sizeof(uint16_t);

    uint16_t class = ntohs(*(uint16_t*)dns_buf_p);
    dns_buf_p += sizeof(uint16_t);

    uint32_t ttl = ntohl(*(uint32_t*)dns_buf_p);
    dns_buf_p += sizeof(uint32_t);

    uint16_t rdlength = ntohs(*(uint16_t*)dns_buf_p);
    dns_buf_p += sizeof(uint16_t);

    printf("Type: %d, class: %d, ttl; %d, rdlen: %d\n", type, class, ttl, rdlength);

    // Parse rdata
    uint32_t ip = *(uint32_t*)dns_buf_p;

    char ip_str[INET_ADDRSTRLEN];
    const char* r = inet_ntop(AF_INET, &ip, ip_str, sizeof(ip_str));
    if (!r) {
        fprintf(stderr, "Coukd not parse address pton\n");
        return -1;
    }
    printf("Address: %s\n", ip_str);
    return 0;
}


int main() {
    // TODOS:
    // 1. Refactor send functions [DONE]; Refactor receive func {NOT DONE}
    // 2. Ability to parse several answers (LIKE getaddrinfo) {NOT DONE}
    // 3. Figure out records {NOT DONE}

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    int ret = inet_pton(AF_INET, GOOGLE_DNS, &addr.sin_addr); // Google dns
    if (ret < 0) {
        perror("inet_pton failed");
        return -1;
    }

    ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr)); // since now i can use send instead of sendto
    if (ret < 0) {
        perror("Connect failed");
        return -1;
    }


    ret = send_dns_request(sock, "www.vk.ru");
    if (ret < 0) {
        perror("send dns request failed");
        return -1;
    }

    ret = parse_dns_response(sock);
    if (ret < 0) {
        perror("parse dns resp failed");
        return -1;
    }


    // close(sock);
    return 0;
}




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
