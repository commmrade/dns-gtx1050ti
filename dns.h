#ifndef __DNS_GUARD
#define __DNS_GUARD

#include <stdint.h>

#define GOOGLE_DNS "8.8.8.8"

#define QR_FLAG (1 << 15)
#define AA_BIT (1 << 10)
#define TRUNC_BIT (1 << 9)
#define RD_BIT (1 << 8)
#define RA_BIT (1 << 7)
#define RCODE_MASK (0b0000000000001111)

#define IPV4_RECORD 0x01
#define IPV6_RECORD 28


struct __attribute__((packed)) dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

struct dns_question {
    // uint8_t* qname;
    uint16_t qtype;
    uint16_t qclass;
} __attribute__((packed));

struct dns_answer {
    // uint8_t* name;
    uint16_t type;
    uint16_t class_;
    uint32_t ttl;
    uint16_t rdlength;
    // uint8_t* rdata; // Same length as rdlength
} __attribute__((packed));



int send_dns_request(int sock, int qtype, const char* domain);
int parse_dns_response(int sock);


#endif
