#include "utils.h"
#include <stdlib.h>
#include <string.h>

char* process_domain(const char* domain) {
    if (!domain) return NULL;

    char* result_buf = (char*)malloc(MAX_DOMAIN_LENGTH);
    int result_offset = 0;

    const char* base = domain;
    const char* found;
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
    return result_buf;
}
