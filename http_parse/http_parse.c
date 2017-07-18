#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_common_def.h"
#include "http_parse.h"

static http_t* http_parse_message = NULL;	// 解析后的数据放在此结构体中
static char tcp_receive_buffer[MAX_TCP_RECEIVE_BUFFER_LENGTH] = { 0 };	// 解析过程中的存放数据的buffer
static int buffer_end_position = 0;	// buffer长度/buffer待拷贝位置
static char header_buffer[MAX_LINE_LENGTH] = { 0 };	// 解析状态行/请求行需要的buffer
static int bs_message_first_parse = HTTP_TRUE;	// 控制每次报文解析时候的头一行
static int bs_to_init_flag = HTTP_TRUE;

void http_parse_init();
void http_error_handle();
void set_http_header_line_config(const char* pname, const char* pvalue);
void set_http_method_line_config(const char* input_string, int end_pos);

//初始化解析过程中用到的参数
void http_parse_init(){
	buffer_end_position = 0;
	memset(tcp_receive_buffer, 0 ,MAX_TCP_RECEIVE_BUFFER_LENGTH);
	memset(header_buffer, 0 ,MAX_LINE_LENGTH);
	bs_message_first_parse = HTTP_TRUE;
	if(http_parse_message == NULL){
		http_parse_message = (http_t*) malloc(sizeof(http_t));
		if(http_parse_message){
			memset(http_parse_message, 0, sizeof(http_t));
			http_parse_message->http_current_state = PARSING;
		}else{
			printf("malloc memory failed!\n");
		}
	}else{
			memset(http_parse_message, 0, sizeof(http_t));
			http_parse_message->http_current_state = PARSING;
	}
}

// 错误处理
void http_error_handle(){
	printf( "Error happened!");
	// 此处你需要依赖平台终止进程/线程/任务
}

// 在 input_string 中查找字符c,找到则返回第一次的位置，否则返回 -1
int split_data_by_delimiter(const char* input_string, int length, char c){
	for(int i = 0; i < length; ++i){
		if(input_string[i] == c)
			return i;
	}
	printf("error!cannot find split delimiter '%c'",c);
	return -1;
}

// 解析 HTTP 报文的第一行
void set_http_method_line_config(const char* input_string, int end_pos){	// length:second + 1
	memcpy(header_buffer, input_string,end_pos + 1);
	header_buffer[end_pos+2] = 0;
	int first_space_position = split_data_by_delimiter(input_string,end_pos,' ');
	int second_space_position = split_data_by_delimiter(input_string + first_space_position + 1, end_pos - first_space_position,' ');
	second_space_position = first_space_position + second_space_position + 1;
	const char* p_head_buffer =(const char*)header_buffer;
	if(strstr(p_head_buffer,"GET")){	// GET 请求
		printf("set a GET command");
		http_parse_message->mfi_http_state_bs.http_method = 0;
		memcpy(http_parse_message->mfi_http_header.http_method,"GET",sizeof("GET"));
		memcpy(http_parse_message->mfi_http_header.request_url,p_head_buffer + first_space_position + 1, second_space_position - first_space_position - 1);
		memcpy(http_parse_message->mfi_http_header.http_version, p_head_buffer + second_space_position + 1, end_pos - second_space_position);
		http_parse_message->mfi_http_state_bs.http_version_flag = 1;
		http_parse_message->mfi_http_state_bs.request_url_flag = 1;
	}else if(strstr(p_head_buffer,"POST")){	// POST 请求
		printf("set a POST command");
		http_parse_message->mfi_http_state_bs.http_method = 2;
		memcpy(http_parse_message->mfi_http_header.http_method,"POST",sizeof("POST"));
		memcpy(http_parse_message->mfi_http_header.request_url,p_head_buffer + first_space_position + 1, second_space_position - first_space_position);
		memcpy(http_parse_message->mfi_http_header.http_version, p_head_buffer + second_space_position + 1, end_pos - second_space_position);
		http_parse_message->mfi_http_state_bs.http_version_flag = 1;
		http_parse_message->mfi_http_state_bs.request_url_flag = 1;
	}else if(strstr(p_head_buffer,"PUT")){	// PUT 请求
		printf("set a PUT command");
		http_parse_message->mfi_http_state_bs.http_method = 3;
		memcpy(http_parse_message->mfi_http_header.http_method,"PUT",sizeof("PUT"));
		memcpy(http_parse_message->mfi_http_header.request_url,p_head_buffer + first_space_position + 1, second_space_position - first_space_position);
		memcpy(http_parse_message->mfi_http_header.http_version, p_head_buffer + second_space_position + 1, end_pos - second_space_position);
		http_parse_message->mfi_http_state_bs.http_version_flag = 1;
		http_parse_message->mfi_http_state_bs.request_url_flag = 1;
	}else if(strstr(p_head_buffer,"HTTP/1.1")){	// 回复 GET
		printf("set a RESPOND command");
		http_parse_message->mfi_http_state_bs.http_method = 1;
		memcpy(http_parse_message->mfi_http_header.http_method,"RESPOND",sizeof("RESPOND"));
		memcpy(http_parse_message->mfi_http_header.http_version,p_head_buffer, first_space_position);
		memcpy(http_parse_message->mfi_http_header.respond_state_code,p_head_buffer + first_space_position + 1, second_space_position - first_space_position - 1);
		memcpy(http_parse_message->mfi_http_header.respond_string, p_head_buffer + second_space_position + 1, end_pos - second_space_position);
		http_parse_message->mfi_http_state_bs.http_version_flag = 1;
		http_parse_message->mfi_http_state_bs.respond_state_code_flag = 1;
		http_parse_message->mfi_http_state_bs.respond_string_flag = 1;
		printf("set respond_state_code_flag:%u\n",http_parse_message->mfi_http_state_bs.respond_state_code_flag);

	}else{
		printf("cannot parse http message header\n");
		http_error_handle();
	}
}

// 解析 HTTP 的其余首部行
void set_http_header_line_config(const char* pname, const char* pvalue){
	printf("set:<pname:%s pvalue:%s>\n",pname,pvalue);
	if(strstr(pname,"Content-Length")){
		http_parse_message->mfi_http_header.content_length = atoi(pvalue);
		http_parse_message->mfi_http_state_bs.content_length_flag = 1;
	}else if(strstr(pname,"Connection")){
		memcpy(http_parse_message->mfi_http_header.http_connection, pvalue, strlen(pvalue));
		http_parse_message->mfi_http_state_bs.http_connection_flag = 1;
	}else if(strstr(pname,"Host")){
		memcpy(http_parse_message->mfi_http_header.http_host, pvalue, strlen(pvalue));
		http_parse_message->mfi_http_state_bs.http_host_flag = 1;
	}else if(strstr(pname,"Content-Type")){
		memcpy(http_parse_message->mfi_http_header.content_type, pvalue, strlen(pvalue));
		http_parse_message->mfi_http_state_bs.content_type_flag = 1;
	}else{
		printf("IGNORE current config!current name:%s current value:%s", pname, pvalue);
	}
}

// 打印数组 parse_line，从 start_position 打印到 end_position， 会附带标记字符串 p
// 分别 %d 和 %c 打印当前数组内容
// 会打印当前数组长度
// %d打印提供格式16字节对齐
void print_memory(char parse_line[], int start_position, int end_position,const char* p){
	printf("<%s>print %%d memory(%dB):\n",p,end_position - start_position + 1);
	int count = 0;
	for(int i = start_position; i <= end_position; ++ i){
		printf("%d ",parse_line[i]);
		count++;
		if(parse_line[i] == 0){
			break;
		}
		if(count % 16 == 0)
			printf("\n");
	}
	printf("\n");

	printf("print %%c memory:");
	for(int i = start_position; i <= end_position; ++ i){
		if(parse_line[i] == 0){
			break;
		}
		printf("%c",parse_line[i]);
	}
	printf("\n");
}

// 调用 http_parse 之后，需通过调用 get_current_http_state 来确定当前 HTTP 报文是否结束
// 返回 PARSE_SUCCESS 则解析完毕
// 返回 PARSE_FAILED 则解析失败
// 返回 PARSING 则解析尚未完成
MFI_HTTP_STATE get_current_http_state(){
	return http_parse_message->http_current_state;
}

// 当http_parse解析到 \r\n时候，会将该行的字段通过此接口设置到结构体中
void http_header_parse(char parse_line[], int start_position, int end_position, int first_parse){
	int first_semicolon_position = 0;
	if(first_parse){
		set_http_method_line_config(parse_line,end_position);
	}
	char head_name[16] = { 0 };
	char head_name_value[32] = { 0 };
	first_semicolon_position = split_data_by_delimiter(parse_line + start_position, end_position - start_position + 1, ':');	// 每一行必须符合 RFC 规范，每一行需有 : 分隔符
	memcpy(head_name, parse_line + start_position, first_semicolon_position);
	memcpy(head_name_value, parse_line + start_position + first_semicolon_position + 1, end_position - start_position - first_semicolon_position);
	set_http_header_line_config((const char*)head_name, (const char*)head_name_value);
}

// 将 TCP 层收到的数据，丢到此接口去解析
// 返回 PARSE_SUCCESS 则解析完毕
// 返回 PARSE_FAILED 则解析失败
// 返回 PARSING 则解析尚未完成
MFI_HTTP_STATE http_parse(const char *buffer, int len){

	if(bs_to_init_flag){
		http_parse_init();
	}
	bs_to_init_flag = HTTP_FALSE;
	print_memory(buffer,0,len - 1,"tcp receive message");

	if(len < 0 || http_parse_message->http_current_state != PARSING){
		printf("error:current parse len:%d current state:%d\n",len, http_parse_message->http_current_state);
		http_parse_message->http_current_state = PARSE_FAILED;
		return PARSE_FAILED;
	}
	// 如果长度为 0 ，则报文结束
	if(len == 0){
		printf("http message over!");
		if(http_parse_message->mfi_http_state_bs.content_length_flag == 0 && http_parse_message->mfi_http_state_bs.http_header_flag){	// HTTP 不存在 body
			printf("parse success,current message not exist content/http body\n");
			http_parse_message->http_current_state = PARSE_SUCCESS;
			bs_to_init_flag = HTTP_TRUE;
			return PARSE_SUCCESS;
		}else if(http_parse_message->mfi_http_state_bs.content_length_flag){	// HTTP 存在 body
			if(http_parse_message->mfi_http_body.actual_body_length == http_parse_message->mfi_http_header.content_length){
				printf( "message parse over,parse OK!\n");
				http_parse_message->http_current_state = PARSE_SUCCESS;
				bs_to_init_flag = HTTP_TRUE;
				return PARSE_SUCCESS;
			}else if(http_parse_message->mfi_http_state_bs.http_header_flag){
				printf( "OK,receive un-format message over! expect length:%d actual length:%d\n",http_parse_message->mfi_http_header.content_length,http_parse_message->mfi_http_body.actual_body_length);
				http_parse_message->http_current_state = PARSE_SUCCESS;
				bs_to_init_flag = HTTP_TRUE;
				return PARSE_SUCCESS;
			}else{
				printf( "info:need more data to parse header\n");	// http header 尚未收到 \r\n 的结束行
				return PARSING;
			}
		}else{
			printf( "need more data to parse\n");	// http header 尚未收到 \r\n 的结束行
			return PARSING;
		}
	}

	if(len == 1){
		printf( "what the fuck??");
		memcpy(tcp_receive_buffer, buffer, len);
		++buffer_end_position;
		return PARSING;
	}

	// http 头部已经解析完毕
	if(http_parse_message->mfi_http_state_bs.http_header_flag){
		if(http_parse_message->mfi_http_state_bs.content_length_flag){
			if(http_parse_message->mfi_http_body.actual_body_length < http_parse_message->mfi_http_header.content_length){
				memcpy(http_parse_message->mfi_http_body.mfi_http_body + http_parse_message->mfi_http_body.actual_body_length, buffer, len);
				http_parse_message->mfi_http_body.actual_body_length += len;
				if(http_parse_message->mfi_http_header.content_length <= http_parse_message->mfi_http_body.actual_body_length){
					http_parse_message->http_current_state = PARSE_SUCCESS;
					bs_to_init_flag = HTTP_TRUE;
					return PARSE_SUCCESS;
				}
				printf("current body:%s***current actual body:%d\n",http_parse_message->mfi_http_body.mfi_http_body,http_parse_message->mfi_http_body.actual_body_length);
				printf("copy data1 to body,len:%d",len);
				return PARSING;
			}else{
				printf("receive data over!\n");
				http_parse_message->http_current_state = PARSE_SUCCESS;
				bs_to_init_flag = HTTP_TRUE;
				return PARSE_SUCCESS;
			}
		}else{
			printf("no header,receive a message over without http body over\n");
			http_parse_message->http_current_state = PARSE_SUCCESS;
			bs_to_init_flag = HTTP_TRUE;
			return PARSE_SUCCESS;
		}
	}
	int parse_start_position = 0;
	int parse_end_position = 0;
	memcpy(tcp_receive_buffer + buffer_end_position, buffer, len); // 将收到的数据拷贝到 buffer 中去解析
	printf("copy len:%d\n",len);

	// 对 buffer 遍历到尾部，每次找到 \r\n 字样。如果长度为2，则http头结束，否则 解析当前行，如果遍历结束，拷贝剩余待解析字段到buffer,记录剩余长度
	for(int i = 0; i < buffer_end_position + len - 1; ++i){
		if(tcp_receive_buffer[i] == '\r' && tcp_receive_buffer[i+1] == '\n'){
			if(i == parse_start_position){
				// scan /r /n HTTP 头部结束
				printf("scan a http head over flag!!!\n");
				http_parse_message->mfi_http_state_bs.http_header_flag = 1;

				if(http_parse_message->mfi_http_state_bs.content_length_flag){
					if(http_parse_message->mfi_http_body.actual_body_length < http_parse_message->mfi_http_header.content_length){
						int left_body_length = buffer_end_position + len - parse_start_position; // 2 :\r\n
						if(tcp_receive_buffer[i] == '\r'){
							left_body_length = left_body_length - 2;	// copy first body should remove \r\n
						}
						printf("buffer_end_pos:%d len:%d parse_start_pos:%d left_body_length:%d\n",buffer_end_position,len,parse_start_position,left_body_length);
						memcpy(http_parse_message->mfi_http_body.mfi_http_body + http_parse_message->mfi_http_body.actual_body_length, \
								tcp_receive_buffer + parse_start_position + 2, left_body_length);
						http_parse_message->mfi_http_body.actual_body_length += left_body_length;
						printf("current body:***%s***\nbody length:%d\n",http_parse_message->mfi_http_body.mfi_http_body,http_parse_message->mfi_http_body.actual_body_length);
						if(http_parse_message->mfi_http_header.content_length <= http_parse_message->mfi_http_body.actual_body_length){
							http_parse_message->http_current_state = PARSE_SUCCESS;
							bs_to_init_flag = HTTP_TRUE;
							return PARSE_SUCCESS;
						}
						printf("copy data2 to body,len:%d\n",len);
						return PARSING;
					}else{
						printf("receive data over!\n");
						http_parse_message->http_current_state = PARSE_SUCCESS;
						bs_to_init_flag = HTTP_TRUE;
						return PARSE_SUCCESS;
					}
				}else{
					printf("no body,receive a message over without http body over\n");
					http_parse_message->http_current_state = PARSE_SUCCESS;
					bs_to_init_flag = HTTP_TRUE;
					return PARSE_SUCCESS;
				}
			}
			// 解析到 \r\n 行，准备解析
			parse_end_position = i - 1;
			http_header_parse(tcp_receive_buffer, parse_start_position, parse_end_position,bs_message_first_parse);
			bs_message_first_parse = HTTP_FALSE;
			parse_start_position = i + 2;
			parse_end_position = parse_start_position;
		}
	}
	// 剩余 data 拷贝到 buffer
	buffer_end_position = buffer_end_position + len - parse_start_position;
	for(int i = 0; i < buffer_end_position; ++i)
		tcp_receive_buffer[i] = tcp_receive_buffer[i + parse_start_position];
	return PARSING;
}

const char* get_http_current_method(){
	const char* pret = http_parse_message->mfi_http_header.http_method;
	return pret;
}

const char* get_http_current_url(){
	if(http_parse_message->mfi_http_state_bs.request_url_flag){
		const char* pret = http_parse_message->mfi_http_header.request_url;
		return pret;
	}else{
		printf("NO url config\n");
		return NULL;
	}
}

const char* get_http_current_version(){
	if(http_parse_message->mfi_http_state_bs.http_version_flag){
		const char* pret = http_parse_message->mfi_http_header.http_version;
		return pret;
	}else{
		printf("NO version config\n");
		return NULL;
	}
}

const char* get_http_current_host(){
	if(http_parse_message->mfi_http_state_bs.http_host_flag){
		const char* pret = http_parse_message->mfi_http_header.http_host;
		return pret;
	}else{
		printf("no Host config\n");
		return NULL;
	}
}

const char* get_http_current_content_type(){
	if(http_parse_message->mfi_http_state_bs.content_type_flag){
		const char* pret = http_parse_message->mfi_http_header.content_type;
		return pret;
	}else{
		printf("NO Content-type config\n");
		return NULL;
	}
}

int get_http_current_content_length(){
	if(http_parse_message->mfi_http_state_bs.content_length_flag){
		int pret = http_parse_message->mfi_http_header.content_length;
		return pret;
	}else{
		printf("NO Content-length config\n");
		return NULL;
	}
}

const char* get_http_current_state_code(){
	if(http_parse_message->mfi_http_state_bs.respond_state_code_flag){
		const char* pret = http_parse_message->mfi_http_header.respond_state_code;
		return pret;
	}else{
		printf("NO state-code config\n");
		return NULL;
	}
}

const char* get_http_current_state_string(){
	if(http_parse_message->mfi_http_state_bs.respond_string_flag){
		const char* pret = http_parse_message->mfi_http_header.respond_string;
		return pret;
	}else{
		printf("NO state-string config\n");
		return NULL;
	}
}

const char* get_http_current_connection(){
	if(http_parse_message->mfi_http_state_bs.http_connection_flag){
		const char* pret = http_parse_message->mfi_http_header.http_connection;
		return pret;
	}else{
		printf("NO connection config\n");
		return NULL;
	}
}

// 获取body内容
const char* get_http_current_body(){
	if(http_parse_message->mfi_http_state_bs.content_length_flag){
		const char* pret = http_parse_message->mfi_http_body.mfi_http_body;
		return pret;
	}else{
		printf("NO body config\n");
		return NULL;
	}
}



// 可通过此接口一次性获取所有解析后的字段。
// 如果 不想获取该字段，则 设置 NULL
const char* get_http_parse_result(char** method,char** url,char** version,char** state_code,\
		const char** state_string,const char** content_type,int* content_length,const char** connection,const char** host,const char** data){
	if(method && http_parse_message->mfi_http_state_bs.http_method){
		*method = get_http_current_method();
	}
	if(url && http_parse_message->mfi_http_state_bs.request_url_flag){
		*url = get_http_current_url();
	}
	if(version && http_parse_message->mfi_http_state_bs.http_version_flag){
		*version = get_http_current_version();
	}
	if(state_code && http_parse_message->mfi_http_state_bs.respond_state_code_flag){
		*state_code = get_http_current_state_code();
	}
	if(state_string && http_parse_message->mfi_http_state_bs.respond_string_flag){
		*state_string = get_http_current_state_string();
	}
	if(content_type && http_parse_message->mfi_http_state_bs.content_type_flag){
		*content_type = get_http_current_content_type();
	}
	if(content_length && http_parse_message->mfi_http_state_bs.content_length_flag){
		*content_length = get_http_current_content_length();
	}
	if(connection && http_parse_message->mfi_http_state_bs.http_connection_flag){
		*connection = get_http_current_connection();
	}
	if(host && http_parse_message->mfi_http_state_bs.http_host_flag){
		*host = get_http_current_host();
	}
	if(data && http_parse_message->mfi_http_state_bs.content_length_flag){
		*data = http_parse_message->mfi_http_body.mfi_http_body;
	}
	return NULL;
}

