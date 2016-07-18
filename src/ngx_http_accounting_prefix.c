#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "ngx_http_accounting_prefix.h"

u_int find_end_of_path_part(u_char *start, ngx_http_request_t *r);

ngx_str_t extract_routing_prefix(ngx_http_request_t *r)
{
    u_char *start = r->uri.data;

    // We consider any request that either has an empty uri or does not start with a '/' to be malformed
    if (start == NULL || *start != '/')
    {
        return (ngx_str_t) {17, (u_char *)"malformed-request"};
    }
    // We have established that the URI starts with a slash
    ++start;

    // We need to consider 4 cases before we identify the namespace: root, 'rest', 'addons', 'private'
    char *schemes[] = {"rest", "addons", "private"};
    u_int num_schemes = 3;

    u_int len = find_end_of_path_part(start, r);

    if (len == 0)
    {
        return (ngx_str_t) {7, (u_char *)"default"};
    }

    ngx_str_t firstpart = (ngx_str_t) {len, start};
    u_int i;
    for (i = 0; i < num_schemes; ++i)
    {
        if (strncmp(schemes[i], (char *)firstpart.data, len) == 0)
        {
            // we've found a path schema, need to find the namespace
            start += len;
            if ( *start == '/')
            {
                ++start;
                return (ngx_str_t) {find_end_of_path_part(start, r), start};
            }
        }
    }


    // We return the desired length of the accounting_id along with a pointer to the first character in the
    // accounting_id, i.e. the char after the first '/'.
    return (ngx_str_t) {len, start};
}

u_int find_end_of_path_part(u_char *start, ngx_http_request_t *request)
{
    // We allow alphanumeric characters, dashes and underscores in path parts. We will use anything
    // between the first slash and the next forbidden character(usually slash or whitespace) as the accounting_id
    u_char *cmp = start;
    u_int idx = 0;
    while (  idx < request->uri.len-1 // uri.len includes the preceding slash which we want to ignore
             && (isalnum(*cmp) || *cmp == '_' || *cmp == '-'))
    {
        ++idx;
        ++cmp;
    }
    return idx;
}