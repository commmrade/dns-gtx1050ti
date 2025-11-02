#ifndef __UTILS_GUARD
#define __UTILS_GUARD

#define MAX_DOMAIN_LENGTH 255 // 253 + padding j in case
#define MAX_DNS_LENGTH 512

char* process_domain(const char* domain);

#endif
