#include "http_group.h"
#include "http_common_def.h"

static char http_send_byte_stream[MAX_HTTP_SEND_BUFFER_LENGTH] = { 0 };	// 组好的报文数据流放在此buffer中
static http_t* http_group_message = NULL;		// 待发送的数据放在此结构体中

void http_group_init();
int group_get_message();
int group_respond_message();
int group_post_message();
int group_put_message();

// group package
const char* esp_mfi_http_group_package(const char* method,const char* url,const char* version,const char* state_code,\
		const char* state_string,const char* content_type,int content_length,const char* connection,const char* host,const char* data){
	http_group_init();
	int data_length = content_length;	// 默认HTTP头的content-length字段 符合 数据真实长度
	if(method){
		set_http_method(method,strlen(method));
	}

	if(url){
		set_http_url(url,strlen(url));
	}
	if(version){
		set_http_version(version,strlen(version));
	}
	if(state_code){
		set_http_state_code(state_code,strlen(state_code));
	}
	if(state_string){
		set_http_state_string(state_string,strlen(state_string));
	}
	if(content_type){
		set_http_content_type(content_type,strlen(content_type));
	}
	if(content_length > 0){
		set_http_content_length(content_length);
	}
	if(connection){
		set_http_connection(connection,strlen(connection));
	}
	if(host){
		set_http_host(host,strlen(host));
	}

	if(data_length > 0 && data != NULL){
		set_http_body_content(data,data_length);
	}

	MFI_HTTP_METHOD method_id = MFI_HTTP_INIT;
	if(strstr(method,"GET")){
		method_id = MFI_HTTP_GET;
	}else if(strstr(method,"RESPOND")){
		method_id = MFI_HTTP_RESPOND;
	}else if(strstr(method,"POST")){
		method_id = MFI_HTTP_POST;
	}else if(strstr(method,"PUT")){
		method_id = MFI_HTTP_PUT;
	}else{
		printf("uncontrol method!");
	}

	switch(method_id){
	case MFI_HTTP_GET:
		if(group_http_message(MFI_HTTP_GET)){
			return http_send_byte_stream;
		}
		break;
	case MFI_HTTP_RESPOND:
		if(group_http_message(MFI_HTTP_RESPOND)){
			return http_send_byte_stream;
		}
		break;
	case MFI_HTTP_POST:
		if(group_http_message(MFI_HTTP_POST)){
			return http_send_byte_stream;
		}
		break;
	case MFI_HTTP_PUT:
		if(group_http_message(MFI_HTTP_PUT)){
			return http_send_byte_stream;
		}
		break;
	default :
		return NULL;
	}
    return NULL;
}

void http_group_init(){
	memset(http_send_byte_stream,0,MAX_HTTP_SEND_BUFFER_LENGTH);
	if(http_group_message == NULL){
		http_group_message = (http_t*)malloc(sizeof(http_t));
		if(http_group_message){
			memset(http_group_message, 0, sizeof(http_t));
		}else{
			printf("failed to malloc memory!");
		}
	}else{
			memset(http_group_message, 0, sizeof(http_t));
	}
}


// group MESSAGE
void set_http_method(const char* pmethod,int length){

	memcpy(http_group_message->mfi_http_header.http_method,pmethod, length);
	http_group_message->mfi_http_state_bs.http_method = 1;
}

void set_http_url(const char* purl,int length){
	memcpy(http_group_message->mfi_http_header.request_url,purl, length);
	http_group_message->mfi_http_state_bs.request_url_flag = 1;
}

void set_http_version(const char* pversion,int length){
	memcpy(http_group_message->mfi_http_header.http_version,pversion, length);
	http_group_message->mfi_http_state_bs.http_version_flag = 1;
}

void set_http_state_code(const char* pstate,int length){
	memcpy(http_group_message->mfi_http_header.respond_state_code,pstate, length);
	http_group_message->mfi_http_state_bs.respond_state_code_flag = 1;
}

void set_http_state_string(const char* pstring,int length){
	memcpy(http_group_message->mfi_http_header.respond_string,pstring, length);
	http_group_message->mfi_http_state_bs.respond_string_flag = 1;
}

void set_http_host(const char* phost,int length){
	memcpy(http_group_message->mfi_http_header.http_host,phost, length);
	http_group_message->mfi_http_state_bs.http_host_flag = 1;
}

void set_http_connection(const char* pconnection,int length){
	memcpy(http_group_message->mfi_http_header.http_connection,pconnection, length);
	http_group_message->mfi_http_state_bs.http_connection_flag = 1;
}

void set_http_content_type(const char* pcontent_type,int length){
	memcpy(http_group_message->mfi_http_header.content_type,pcontent_type, length);
	http_group_message->mfi_http_state_bs.content_type_flag = 1;
}

void set_http_content_length(int content_length){
	http_group_message->mfi_http_header.content_length = content_length;
	http_group_message->mfi_http_state_bs.content_length_flag = 1;
}

void set_http_body_content(const char* pbody,int length){
	memcpy(http_group_message->mfi_http_body.mfi_http_body, pbody, length);
	http_group_message->mfi_http_body.actual_body_length = length;
}

const char* get_http_group_current_byte_stream(){
	return http_send_byte_stream;
}

HTTP_FLAG group_get_message(){
	if(http_group_message->mfi_http_state_bs.http_method == 0 || \
			http_group_message->mfi_http_state_bs.request_url_flag == 0 || http_group_message->mfi_http_state_bs.http_version_flag == 0){
		return 0;
	}
	sprintf(http_send_byte_stream,"GET %s %s\r\n",http_group_message->mfi_http_header.request_url, http_group_message->mfi_http_header.http_version);

}


HTTP_FLAG group_respond_message(){
	if(http_group_message->mfi_http_state_bs.respond_string_flag == 0 || \
			http_group_message->mfi_http_state_bs.respond_state_code_flag == 0 || \
			http_group_message->mfi_http_state_bs.http_version_flag == 0){
		return HTTP_FALSE;
	}
	sprintf(http_send_byte_stream,"%s %s %s\r\n",http_group_message->mfi_http_header.http_version, \
			http_group_message->mfi_http_header.respond_state_code,http_group_message->mfi_http_header.respond_string);

}

HTTP_FLAG group_post_message(){
	if(http_group_message->mfi_http_state_bs.http_method == 0 || \
			http_group_message->mfi_http_state_bs.request_url_flag == 0 || \
			http_group_message->mfi_http_state_bs.http_version_flag == 0){
		return HTTP_FALSE;
	}
	sprintf(http_send_byte_stream,"POST %s %s\r\n", \
			http_group_message->mfi_http_header.request_url,http_group_message->mfi_http_header.http_version);

}

HTTP_FLAG group_put_message(){


	if(http_group_message->mfi_http_state_bs.http_method == 0 || \
			http_group_message->mfi_http_state_bs.request_url_flag == 0 || \
			http_group_message->mfi_http_state_bs.http_version_flag == 0){
		return HTTP_FALSE;
	}
	sprintf(http_send_byte_stream,"PUT %s %s\r\n", \
			http_group_message->mfi_http_header.request_url,http_group_message->mfi_http_header.http_version);
	int current_length = strlen(http_send_byte_stream);

	if(http_group_message->mfi_http_state_bs.http_host_flag){
		const char* ptemp = "Host:";
		memcpy(http_send_byte_stream + current_length,ptemp,strlen(ptemp));
		current_length += strlen(ptemp);

		int temp = strlen(http_group_message->mfi_http_header.http_host);
		memcpy(http_send_byte_stream + current_length, http_group_message->mfi_http_header.http_host,temp);
		current_length += temp;
		memcpy(http_send_byte_stream + current_length, "\r\n",2);
		current_length += 2;
	}

	if(http_group_message->mfi_http_state_bs.http_connection_flag){
		const char* ptemp = "Connection:";
		memcpy(http_send_byte_stream + current_length,ptemp,strlen(ptemp));
		current_length += strlen(ptemp);

		int temp = strlen(http_group_message->mfi_http_header.http_connection);
		memcpy(http_send_byte_stream + current_length, http_group_message->mfi_http_header.http_connection,temp);
		current_length += temp;
		memcpy(http_send_byte_stream + current_length, "\r\n",2);
		current_length += 2;
	}

	if(http_group_message->mfi_http_state_bs.content_type_flag){
		const char* ptemp = "Content-Type:";
		memcpy(http_send_byte_stream + current_length,ptemp,strlen(ptemp));
		current_length += strlen(ptemp);

		int temp = strlen(http_group_message->mfi_http_header.content_type);
		memcpy(http_send_byte_stream + current_length, http_group_message->mfi_http_header.content_type,temp);
		current_length += temp;
		memcpy(http_send_byte_stream + current_length, "\r\n",2);
		current_length += 2;
	}

	if(http_group_message->mfi_http_state_bs.content_length_flag){
		const char* ptemp = "Content-Length:";
		memcpy(http_send_byte_stream + current_length,ptemp,strlen(ptemp));
		current_length += strlen(ptemp);

		char temp[5] = { 0 };
		itoa(http_group_message->mfi_http_header.content_length,temp, 10);
		//int temp = strlen(mfi_http_send_buffer->mfi_http_header.content_length);
		memcpy(http_send_byte_stream + current_length, temp,strlen(temp));
		current_length += strlen(temp);
		memcpy(http_send_byte_stream + current_length, "\r\n",2);
		current_length += 2;
	}

	memcpy(http_send_byte_stream + current_length, "\r\n",2);
	current_length += 2;

	// add body
	if(http_group_message->mfi_http_state_bs.content_length_flag > 0){
		memcpy(http_send_byte_stream + current_length, http_group_message->mfi_http_body.mfi_http_body, http_group_message->mfi_http_header.content_length);
		current_length += http_group_message->mfi_http_header.content_length;
	}
	http_group_message->mfi_http_state_bs.http_header_flag = 1;
	return HTTP_TRUE;
}
