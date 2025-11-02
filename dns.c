#include "dns.h"
#include "utils.h"
#include <netinet/in.h>
#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

int send_dns_request(int sock, int qtype, const char* domain) {
    int ret = 0;
    char buf[MAX_DNS_LENGTH];
    char* buf_ptr = buf;
    struct dns_header* dns_hdr = (struct dns_header*)buf_ptr;
    memset(dns_hdr, 0, sizeof(*dns_hdr));
    dns_hdr->id = htons(1337);
   
    uint16_t flags = 0;
    flags |= RD_BIT;
    flags &= ~QR_FLAG;
    dns_hdr->flags = htons(flags);

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

    dns_q->qtype = htons(qtype);
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

    char buf[MAX_DNS_LENGTH];
    char* buf_ptr = buf;
    ssize_t rd_bytes = read(sock, buf, sizeof(buf));
    if (rd_bytes < 0) {
        perror("read error");
        return -1;
    }


    struct dns_header* dns_hdr = (struct dns_header*)buf_ptr;
    buf_ptr += sizeof(*dns_hdr);

    uint16_t flags = ntohs(dns_hdr->flags);
    bool is_recursion_available = (flags & RA_BIT) == RA_BIT;
    bool is_aa = (flags & AA_BIT) == AA_BIT;
    int rcode = (flags & RCODE_MASK);
    if (!is_recursion_available) {
        fprintf(stderr, "Recursion is not available, failing..\n");
        return -1;
    }
    if (rcode != 0) {
        fprintf(stderr, "Server error: %d, failing..\n", rcode);
        return -1;
    }
    // if (!is_aa) {
    //     fprintf(stderr, "Not authoritive answers are not supported\n");
    //     return -1;
    // }

    printf("Questions: %d, Ansers: %d\nRecords: %d, ARecords: %d\n",
        ntohs(dns_hdr->qdcount), ntohs(dns_hdr->ancount), ntohs(dns_hdr->nscount), ntohs(dns_hdr->arcount));

    char* qname = buf_ptr;
    if ((*qname & 0xC0) == 0xC0) {
        fprintf(stderr, "DNS Compression is not supported yet\n");
        return -1;
    }
    size_t qname_len = strlen(qname) + 1;
    buf_ptr += qname_len;

    struct dns_question* dns_q = (struct dns_question*)buf_ptr;
    buf_ptr += sizeof(*dns_q);
    uint16_t qtype = ntohs(dns_q->qtype);
    uint16_t qclass = ntohs(dns_q->qclass);


    int i = 0;
    while (i < ntohs(dns_hdr->ancount)) {
        char* name = buf_ptr;
        if ((*name & 0xC0) == 0xC0) {
            fprintf(stderr, "DNS Compression is not supported yet\n");
            return -1;
        }
        size_t name_len = strlen(name) + 1;
        buf_ptr += name_len;

        struct dns_answer* dns_a = (struct dns_answer*)buf_ptr;
        buf_ptr += sizeof(*dns_a);


        uint16_t type = ntohs(dns_a->type);
        uint16_t class_ = ntohs(dns_a->class_);
        uint32_t ttl = ntohl(dns_a->ttl);
        uint16_t rdlength = ntohs(dns_a->rdlength);

        switch (type) {
            case 1: { // IPv4
                uint32_t ip = *(uint32_t*)buf_ptr;
                buf_ptr += sizeof(uint32_t);

                char ip_str[INET_ADDRSTRLEN];
                const char* r = inet_ntop(AF_INET, &ip, ip_str, sizeof(ip_str));
                if (!r) {
                    fprintf(stderr, "Coukd not parse address pton\n");
                    return -1;
                }
                printf("Address V4: %s\n", ip_str);
                break;
            }
            case 28: { // IPv6
                char* ipv6_buf = buf_ptr; // 16 bytes
                buf_ptr += 16; // ipv6 is 16 bytes (int128)

                char ip_str[INET6_ADDRSTRLEN];
                const char* r = inet_ntop(AF_INET6, &ipv6_buf, ip_str, sizeof(ip_str));
                if (!r) {
                    fprintf(stderr, "Coukd not parse address pton\n");
                    return -1;
                }
                printf("Address V6: %s\n", ip_str);
                break;
            }
        }

        
        ++i;
    }
    return 0;
}
