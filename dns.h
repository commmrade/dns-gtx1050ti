#include <stdint.h>

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
