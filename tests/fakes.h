typedef int ngx_cycle_t;
typedef int ngx_int_t;
typedef struct ngx_str_t {
    int len;
    char * data;
}ngx_str_t;

typedef struct ngx_http_request_t {
    ngx_str_t uri;     
} ngx_http_request_t;

typedef char u_char;
typedef unsigned int u_int;