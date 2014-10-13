#include <ctype.h>
#include <stdlib.h>

#include "ngx_http_accounting_prefix.h"

ngx_str_t extract_routing_prefix(ngx_http_request_t *r)
{

    u_char *start = r->uri.data;

    // We consider any request that either has an empty uri or does not start with a '/' to be malformed
    if (start == NULL || *start != '/') {
        return (ngx_str_t) {17, (u_char *)"malformed-request"};
    }
    ++start;
    u_char *cmp = start;
    u_int len = 0;
    // We allow alphanumeric characters, dashes and underscores in accounting_id. We will use anything 
    // between the first slash and the next forbidden character(usually slash or whitespace) as the accounting_id
    while (len < r->uri.len-1 && (isalnum(*cmp) || *cmp == '_' || *cmp == '-'))
    {
        len++;
        cmp++;
    }
    if (len == 0) {
        return (ngx_str_t) {7, (u_char *)"default"};
    }
    // We return the desired length of the accounting_id along with a pointer to the first character in the
    // accounting_id, i.e. the char after the first '/'.
    return (ngx_str_t) {len, start};
}