#ifndef HTTP_GROUP_H
#define HTTP_GROUP_H

#include "http_common_def.h"

const char* esp_mfi_http_group_package(const char* method,const char* url,const char* version,const char* state_code,\
		const char* state_string,const char* content_type,int content_length,const char* connection,const char* host,const char* data);

void set_http_method(const char* pmethod,int length);
void set_http_url(const char* purl,int length);
void set_http_version(const char* pversion,int length);
void set_http_state_code(const char* pstate,int length);
void set_http_state_string(const char* pstring,int length);
void set_http_host(const char* phost,int length);
void set_http_connection(const char* pconnection,int length);
void set_http_content_type(const char* pcontent_type,int length);
void set_http_content_length(int content_length);
void set_http_body_content(const char* pbody,int length);

const char* get_http_group_current_byte_stream();

#endif
