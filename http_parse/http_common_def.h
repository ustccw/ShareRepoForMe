#ifndef HTTP_COMMON_DEF_H
#define HTTP_COMMON_DEF_H

#define MAX_METHOD_LENGTH 16
#define MAX_REQUEST_URL_LENGTH 1024
#define MAX_VERSION_LENGTH 16
#define MAX_RESPOND_STRING_LENGTH 64
#define MAX_CONTENT_TYPE_LENGTH 64
#define MAX_CONNECTION_LENGTH 16
#define MAX_HOST_LENGTH 64
#define MAX_STATE_CODE 4
#define MAX_CONTENT_LENGTH 4

#define MAX_LINE_LENGTH 512
#define MAX_HTTP_HEADER_LENGTH 2048
#define MAX_HTTP_BODY_LENGTH 4096
#define TOTAL_MESSAGE_LENGTH 8192

#define MAX_TCP_RECEIVE_BUFFER_LENGTH MAX_HTTP_HEADER_LENGTH
#define MAX_HTTP_SEND_BUFFER_LENGTH MAX_HTTP_HEADER_LENGTH

#define TEXT_BUFFSIZE 1024


typedef enum{
	HTTP_FALSE = 0,
	HTTP_TRUE
}HTTP_FLAG;

typedef enum{
	PARSE_FAILED,
	PARSE_SUCCESS,
	PARSING,
}MFI_HTTP_STATE;


typedef enum{
	MFI_HTTP_INIT,
	MFI_HTTP_GET,
	MFI_HTTP_RESPOND,
	MFI_HTTP_POST,
	MFI_HTTP_PUT
}MFI_HTTP_METHOD;

typedef struct esp_mfi_http_header{
	char http_method[MAX_METHOD_LENGTH];
	char request_url[MAX_REQUEST_URL_LENGTH];
	char http_version[MAX_VERSION_LENGTH];
	char respond_state_code[MAX_STATE_CODE];
	char respond_string[MAX_RESPOND_STRING_LENGTH];
	int content_length;
	char content_type[MAX_CONTENT_TYPE_LENGTH];
	char http_connection[MAX_CONNECTION_LENGTH];
	char http_host[MAX_HOST_LENGTH];
}mfi_http_header_t;

typedef struct esp_mfi_http_body{
	char mfi_http_body[MAX_HTTP_BODY_LENGTH];
	int actual_body_length;
}mfi_http_body_t;

// a domain to flag http state
typedef struct mfi_http_bs{
	int http_method ;	 			 // parse(00:GET  01:respond  10:POST  11:PUT) group(0:YES 1:NO)
	int request_url_flag ;			 // 0:NO  1:YES
	int http_version_flag ; 			 // as above
	int respond_state_code_flag ;	 // as above
	int respond_string_flag ;		 // short explain state_code, 0:NO 1:YES
	int http_header_flag ;			 // 0:http header parsing 1:parse done
	int content_length_flag ;		 // 0:NO 1:YES
	int content_type_flag ;			 // as above
	int http_connection_flag ;			 // as above
	int http_host_flag;					 // as above

}esp_mfi_http_bs_t, *p_esp_mfi_http_bs_t;

typedef struct esp_mfi_http_ {

	MFI_HTTP_STATE http_current_state;

	mfi_http_header_t mfi_http_header;

	mfi_http_body_t mfi_http_body;

	esp_mfi_http_bs_t mfi_http_state_bs;

} http_t;


void print_memory(char parse_line[], int start_position, int end_position,const char* p);


#endif
