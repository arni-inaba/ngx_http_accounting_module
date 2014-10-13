
#ifndef TESTING
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#else
#include "../tests/fakes.h"
#endif

ngx_str_t extract_routing_prefix(ngx_http_request_t *r);