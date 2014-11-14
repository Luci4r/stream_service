#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <syslog.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>
#include<sys/types.h>
#include<sys/stat.h>

struct action_data {
	char server_code[15];
        char mobile_code[12];
        char login_time[15];
        char logout_time[15];
	char cur_time[13];
};

struct stream_data {
	char server_code[15];
	char RX_bytes[10];
	char TX_bytes[10];
	char cur_time[13];
};
extern int Debug;
#define DPRINTF(...) do{ if(Debug){\
				printf("[%s:%s:%d]##", __FILE__, __func__, __LINE__);\
				printf(__VA_ARGS__);}\
			 else {\
			 	syslog(LOG_INFO, __VA_ARGS__);\
			 }\
			}while(0)

#define SERVICEPORT 		8889
#define SUCCESS			1
#define RECV_SUCCESS 		SUCCESS
#define RECV_FORMATE_ERROR 	100
#endif /*__GLOBAL_H__*/
