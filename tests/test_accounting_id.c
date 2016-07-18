#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/ngx_http_accounting_prefix.h"

ngx_str_t create_request_and_get_prefix(char uri[], int len);

void test_extract_prefix_from_request_with_only_prefix(void)
{
    char test_location[] = "/prefix";
    char expected_result[] = "prefix";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_prefix_from_request_terminates_on_slash(void)
{
    char test_location[] = "/prefix/something";
    char expected_result[] = "prefix";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    char result_prefix[result.len + 1];
    strncpy(result_prefix, result.data, result.len);
    result_prefix[result.len] = '\0';

    assert(strcmp(expected_result, result_prefix) == 0);
}

void test_extract_prefix_from_request_does_not_terminate_on_dash(void)
{
    char test_location[] = "/prefix-something/else";
    char expected_result[] = "prefix-something";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    char result_prefix[result.len + 1];
    strncpy(result_prefix, result.data, result.len);
    result_prefix[result.len] = '\0';
    assert(strcmp(expected_result, result_prefix) == 0);
}

void test_extract_prefix_from_request_returns_default_on_slash(void)
{
    char test_location[] = "/";
    char expected_result[] = "default";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_prefix_from_request_returns_malformed_request_on_empty(void)
{
    char test_location[] = "";
    char expected_result[] = "malformed-request";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_prefix_from_request_returns_malformed_request_on_null(void)
{
    char *test_location = NULL;
    char expected_result[] = "malformed-request";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_prefix_from_request_returns_malformed_request_if_prefix_does_not_start_with_slash(void)
{
    char *test_location = "something";
    char expected_result[] = "malformed-request";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_namespace_from_request_rest_schema(void)
{
    char test_location[] = "/rest/namespace";
    char expected_result[] =  "namespace";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_prefix_from_request_addons_schema(void)
{
    char test_location[] = "/addons/namespace";
    char expected_result[] =  "namespace";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_prefix_from_request_private_schema(void)
{
    char test_location[] = "/private/namespace";
    char expected_result[] =  "namespace";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

void test_extract_prefix_from_request_rest_schema_without_namespace_returns_empty(void)
{
    char test_location[] = "/rest";
    char expected_result[] =  "";
    ngx_str_t result = create_request_and_get_prefix(test_location, sizeof(test_location));

    assert(strcmp(expected_result, result.data) == 0);
}

ngx_str_t create_request_and_get_prefix(char uri[], int len)
{
    ngx_str_t result;
    ngx_http_request_t request;
    request.uri.data = uri;
    request.uri.len = len;
    result = extract_routing_prefix(&request);
    return result;
}


int main()
{
    test_extract_prefix_from_request_with_only_prefix();
    test_extract_prefix_from_request_terminates_on_slash();
    test_extract_prefix_from_request_does_not_terminate_on_dash();
    test_extract_prefix_from_request_returns_default_on_slash();
    test_extract_prefix_from_request_returns_malformed_request_on_empty();
    test_extract_prefix_from_request_returns_malformed_request_on_null();
    test_extract_prefix_from_request_returns_malformed_request_if_prefix_does_not_start_with_slash();
    test_extract_namespace_from_request_rest_schema();
    test_extract_prefix_from_request_addons_schema();
    test_extract_prefix_from_request_private_schema();
    test_extract_prefix_from_request_rest_schema_without_namespace_returns_empty();
    printf("Tests passed!\n");
    return 0;
}
