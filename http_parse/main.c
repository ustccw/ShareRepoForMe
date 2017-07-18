#include "string.h"

#include "http_common_def.h"
#include "http_parse.h"
#include "http_group.h"

#define EXAMPLE_SERVER_IP "202.89.233.103"
#define EXAMPLE_SERVER_PORT "80"
int socket_id = -1;
char http_request[64] = {0};
char text[BUFFSIZE + 1] = { 0 };
bool connect_to_http_server();

void http_main_error_handle(){
	printf("some error happened!");
}

void test_parse_message(const char* p){
	// 解析报文
#if 1
	   if (connect_to_http_server()) {
	        printf( "Connected to http server");
	    } else {
	        printf( "Connect to http server failed!");
	    }
	    int res = -1;
	    /*send GET request to http server*/
	    res = send(socket_id, p, strlen(p), 0);
	    if (res == -1) {
	        printf( "Send GET request to server failed");
	        //task_fatal_error();
	    } else {
	        printf( "Send GET request to server succeeded");
	    }
#endif

	printf("start parse message:\n");

#if 1

	while(1){
		http_t *http = NULL;
		int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
		MFI_HTTP_STATE http_state = http_parse(text, buff_len);
		if(http_state == PARSING){
			continue;
		}else if(http_state == PARSE_SUCCESS){
			break;
		}else{
			printf("parse message failed!!");
			break;
		}
	}
	printf("after this parse,http state:%d\n",get_current_http_state());
#endif

}


bool connect_to_http_server()
{
    printf( "Server IP: %s Server Port:%s", EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);
//    sprintf(http_request, "GET %s HTTP/1.1\r\nHost: %s:%s \r\n\r\n", EXAMPLE_FILENAME, EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);

    int  http_connect_flag = -1;
    struct sockaddr_in sock_info;

    socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        printf( "Create socket failed!");
        return false;
    }

    // set connect info
    memset(&sock_info, 0, sizeof(struct sockaddr_in));
    sock_info.sin_family = AF_INET;
    sock_info.sin_addr.s_addr = inet_addr(EXAMPLE_SERVER_IP);
    sock_info.sin_port = htons(atoi(EXAMPLE_SERVER_PORT));

    // connect to http server
    http_connect_flag = connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
    if (http_connect_flag == -1) {
        printf( "Connect to server failed! errno=%d", errno);
        close(socket_id);
        return false;
    } else {
        printf( "Connected to server");
        return true;
    }
    return false;
}


void hello_task(void *pvParameter)
{

}

int main()
{
	char* p = NULL;
// group pakcet
	p = esp_mfi_http_group_package("GET","/","HTTP/1.1",0,0,0,0,0,0,0);
	print_memory(p,0,strlen(p),"print send request:");
	test_parse_message(p);
	char* method = NULL;
	char* version = NULL;
	char* state_code = NULL;
	char* state_string = NULL;
	char* url = NULL;
	char* type = NULL;
	char* connection = NULL;
	char* host = NULL;
	char* data = NULL;
	int length = NULL;

	get_http_parse_result(&method, 0, &version, &state_code, &state_string, 0, 0, 0, 0 ,0);

	printf("\n\ntag:method:%s version:%s state_code:%s state_string:%s\n\n\n",method,version,state_code,state_string);

	//test post message
	p = esp_mfi_http_group_package("POST","/posturl","HTTP/1.1","200","OK","CCCposttype",10,"connectpost","hostpost","1234567890");
	http_parse(p,strlen(p));
	get_http_parse_result(&method, &url, &version, 0, 0, &type, &length, &connection, &host ,&data);
	printf("\n\ntag:method:%s version:%s url:%s type:%s len:%d connection:%s host:%s data:%s\n\n\n",\
			method,version,url,type,length,connection,host,data);

	//test post message
	p = esp_mfi_http_group_package("PUT","/PUTurl","HTTP/1.1","200","OK","CCCpUTtype",14,"connectPUT","hostPUT","1357924680ABCD");
	http_parse(p,strlen(p));
	get_http_parse_result(&method, &url, &version, 0, 0, &type, &length, &connection, &host ,&data);
	printf("\n\ntag:method:%s version:%s url:%s type:%s len:%d connection:%s host:%s data:%s\n\n\n",\
			method,version,url,type,length,connection,host,data);


    printf("task over,task would delete...");
    return 1;
}
