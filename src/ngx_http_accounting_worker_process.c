#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <syslog.h>
#include <stdlib.h>

#include "ngx_http_accounting_hash.h"
#include "ngx_http_accounting_module.h"
#include "ngx_http_accounting_common.h"
#include "ngx_http_accounting_status_code.h"
#include "ngx_http_accounting_worker_process.h"


static ngx_event_t  write_out_ev;
static ngx_http_accounting_hash_t  stats_hash;

static ngx_int_t ngx_http_accounting_old_time = 0;
static ngx_int_t ngx_http_accounting_new_time = 0;

static ngx_uint_t worker_process_interval = 10;

static u_char *ngx_http_accounting_title = (u_char *)"NgxAccounting";

static void worker_process_alarm_handler(ngx_event_t *ev);
static ngx_str_t *get_accounting_id(ngx_http_request_t *r);


ngx_int_t
ngx_http_accounting_worker_process_init(ngx_cycle_t *cycle)
{
    ngx_int_t rc;
    ngx_time_t  *time;
    ngx_http_accounting_main_conf_t *amcf;

    amcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_accounting_module);

    if (!amcf->enable) {
        return NGX_OK;
    }

    init_http_status_code_map();

    time = ngx_timeofday();

    ngx_http_accounting_old_time = time->sec;
    ngx_http_accounting_new_time = time->sec;

    openlog((char *)ngx_http_accounting_title, LOG_NDELAY, LOG_SYSLOG);

    rc = ngx_http_accounting_hash_init(&stats_hash, NGX_HTTP_ACCOUNTING_NR_BUCKETS, cycle->pool);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_memzero(&write_out_ev, sizeof(ngx_event_t));

    write_out_ev.data = NULL;
    write_out_ev.log = cycle->log;
    write_out_ev.handler = worker_process_alarm_handler;

    worker_process_interval = amcf->interval;
    
    srand(ngx_getpid());
    ngx_add_timer(&write_out_ev, worker_process_interval*(1000-rand()%200));

    return NGX_OK;
}


void ngx_http_accounting_worker_process_exit(ngx_cycle_t *cycle)
{
    ngx_http_accounting_main_conf_t *amcf;

    amcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_accounting_module);

    if (!amcf->enable) {
        return;
    }

    worker_process_alarm_handler(NULL);
}


ngx_int_t
ngx_http_accounting_handler(ngx_http_request_t *r)
{
    ngx_str_t      *accounting_id;
    ngx_uint_t      key;

    ngx_uint_t      status;
    ngx_uint_t     *status_array;

    ngx_http_accounting_stats_t *stats;

    accounting_id = get_accounting_id(r);

    // TODO: key should be cached to save CPU time
    key = ngx_hash_key_lc(accounting_id->data, accounting_id->len);
    stats = ngx_http_accounting_hash_find(&stats_hash, key, accounting_id->data, accounting_id->len);

    if (stats == NULL) {

        stats = ngx_pcalloc(stats_hash.pool, sizeof(ngx_http_accounting_stats_t));
        status_array = ngx_pcalloc(stats_hash.pool, sizeof(ngx_uint_t) * http_status_code_count);

        if (stats == NULL || status_array == NULL)
            return NGX_ERROR;

        stats->http_status_code = status_array;
        ngx_http_accounting_hash_add(&stats_hash, key, accounting_id->data, accounting_id->len, stats);
    }

    if (r->err_status) {
        status = r->err_status;
    } else if (r->headers_out.status) {
        status = r->headers_out.status;
    } else {
        status = NGX_HTTP_DEFAULT;
    }

    stats->nr_requests += 1;
    stats->bytes_in += r->request_length;
    stats->bytes_out += r->connection->sent;
    stats->http_status_code[http_status_code_to_index_map[status]] += 1;

    return NGX_OK;
}


static ngx_int_t
worker_process_write_out_stats(u_char *name, size_t len, void *val, void *para1, void *para2)
{
    ngx_uint_t   i;
    ngx_http_accounting_stats_t  *stats;

    char output_buffer[1024];

    ngx_uint_t status_code_buckets[10];
    ngx_memzero(status_code_buckets, sizeof(status_code_buckets));

    stats = (ngx_http_accounting_stats_t *)val;

    if (stats->nr_requests == 0) {
        return NGX_OK;
    }

    for (i = 0; i < http_status_code_count; i++) {
        ngx_uint_t bucket_idx = index_to_http_status_code_map[i] / 100;
        status_code_buckets[bucket_idx] += stats->http_status_code[i];
        stats->http_status_code[i] = 0;
    }

    sprintf(output_buffer, "pid:%i|from:%ld|to:%ld|accounting_id:%s|requests:%ld|bytes_in:%ld|bytes_out:%ld|2xx:%lu|4xx:%lu|5xx:%lu",
                ngx_getpid(),
                ngx_http_accounting_old_time,
                ngx_http_accounting_new_time,
                name,
                stats->nr_requests,
                stats->bytes_in,
                stats->bytes_out,
                status_code_buckets[2],
                status_code_buckets[4],
                status_code_buckets[5]
            );

    stats->nr_requests = 0;
    stats->bytes_out = 0;

    syslog(LOG_INFO, "%s", output_buffer);

    return NGX_OK;
}


static void
worker_process_alarm_handler(ngx_event_t *ev)
{
    ngx_time_t  *time;
    ngx_msec_t   next;

    time = ngx_timeofday();

    ngx_http_accounting_old_time = ngx_http_accounting_new_time;
    ngx_http_accounting_new_time = time->sec;

    ngx_http_accounting_hash_iterate(&stats_hash, worker_process_write_out_stats, NULL, NULL);

    if (ngx_exiting || ev == NULL)
        return;

    next = (ngx_msec_t)worker_process_interval * 1000;

    ngx_add_timer(ev, next);
}


static ngx_str_t *
get_accounting_id(ngx_http_request_t *r)
{
    ngx_http_accounting_loc_conf_t  *alcf;

    alcf = ngx_http_get_module_loc_conf(r, ngx_http_accounting_module);

    return &alcf->accounting_id;
}
