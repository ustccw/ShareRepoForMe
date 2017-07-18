#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

#include "http_common_def.h"

const char* get_http_parse_result(char** method,char** url,char** version,char** state_code,\
		const char** state_string,const char** content_type,int* content_length,const char** connection,const char** host,const char** data);

const char* get_http_current_method();
const char* get_http_current_url();
const char* get_http_current_version();
const char* get_http_current_host();
const char* get_http_current_content_type();
int   get_http_current_content_length();
const char* get_http_current_state_code();
const char* get_http_current_state_string();
const char* get_http_current_connection();

const char* get_http_current_body();

MFI_HTTP_STATE get_current_http_state();

MFI_HTTP_STATE http_parse(const char *buffer, int len);

#endif
