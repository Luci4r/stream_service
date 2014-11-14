#include "global.h"
#include "mysql_save.h"

int Debug = 0;
#define BUFLEN 100
void daemonize(const char *cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	openlog(cmd, LOG_CONS, LOG_DAEMON);

	umask(0);
	if(getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		DPRINTF("%s: can't get file limit\n", cmd);
		goto err;
	}

	if((pid = fork()) < 0) {
		DPRINTF("%s: can't fork\n", cmd);
		goto err;
	} else if(pid != 0)
		exit(0);
	setsid();

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL) < 0) {
		DPRINTF("%s: can't ignore SIGHUP\n", cmd);
		goto err;
	}
	if((pid = fork()) < 0) {
		DPRINTF("%s: can't fork second\n", cmd);
		goto err;
	} else if(pid != 0)
		exit(0);
	
	if(chdir("/") < 0) {
		DPRINTF("%s: can't change directory to /\n", cmd);
		goto err;
	}

	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for(i = 0; i < rl.rlim_max; i++)
		close(i);
	
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	if(fd0 != 0 || fd1 != 1 || fd2 != 2) {
		DPRINTF("%s: unexpected file descriptors %d, %d, %d\n", cmd, fd0, fd1, fd2);
		goto err;
	}

	return;

err:
	syslog(LOG_ERR, "%s starting faild!\n", cmd);

	exit(1);
}

struct action_data *decode_data_action(char *buf)
{
	time_t cur_tm;
	struct action_data *decode_data;
	struct tm *s_time;
	char *p = NULL;
	char *q = NULL;
	time(&cur_tm);
	s_time = localtime(&cur_tm);
	decode_data = (struct action_data *)malloc(sizeof(struct action_data));
	if(decode_data == NULL) {
		DPRINTF("data malloc error\n");
		return NULL;
	}
	memset(decode_data, 0, sizeof(struct action_data));
	/* TODO: add range judgement
	p = strchr(buf, '+');
	data_len = atoi(p);
	if(data_len >100 || data_len < 0) {
		DPRINTF("data len error %d\n", data_len);
		return NULL;
	}*/
	p = strchr(buf + 4, '+');
	q = strchr(p + 1, '+');
	snprintf(decode_data->server_code, q - p,  "%s", p + 1);
	p = strchr(q + 1, '+');
	snprintf(decode_data->mobile_code, p - q, "%s", q + 1);
	q = strchr(p + 1, '+');
	snprintf(decode_data->login_time, q - p, "%s", p + 1);
	p = strchr(q + 1, '=');
	snprintf(decode_data->logout_time, p - q, "%s", q + 1);
	snprintf(decode_data->cur_time, 13, "%04d%02d%02d%02d%02d", s_time->tm_year + 1900, s_time->tm_mon + 1, s_time->tm_mday, s_time->tm_hour,s_time->tm_min);
	return decode_data;
}

void decode_data_bytes(char *data)
{
	struct stream_data *stream_data;
	time_t cur_tm;
	struct tm *s_time;
	time(&cur_tm);
	s_time = localtime(&cur_tm);
	stream_data = (struct stream_data *)malloc(sizeof(struct stream_data));
	memset(stream_data, 0, sizeof(struct stream_data));
	DPRINTF("bytes data is =ll%s\n", data);
	char *p = NULL;
	char *q = NULL;
	p = strchr(data, '+');
	q = strchr(p + 1, '+');
	snprintf(stream_data->server_code, q - p,  "%s", p + 1);
	p = strchr(q + 1, '+');
	snprintf(stream_data->RX_bytes, p - q, "%s", q + 1);
	q = strchr(p + 1, '=');
	snprintf(stream_data->TX_bytes, q - p, "%s", p + 1);
	snprintf(stream_data->cur_time, 13, "%04d%02d%02d%02d%02d", s_time->tm_year + 1900, s_time->tm_mon + 1, s_time->tm_mday,s_time->tm_hour, s_time->tm_min);
	save_stream_data(stream_data);
	free(stream_data);
}

int data_rec(int sock, char *buf)
{
	int new_sock = sock;
	int num_rev = -1;
	num_rev = recv(new_sock, buf, BUFLEN, 0);
	if(num_rev == 0) {
		DPRINTF("connection is closed by client\n");
		return -1;
	} else if(num_rev < 0) {
		DPRINTF("error while receive message\n");
		return -2;
	}
	return num_rev;
}
int judge_data_type(char *buf, int num_recv)
{
	if(num_recv < 4) {
		DPRINTF("data formate error\n");
		return -1;
	}
	if(strncmp(buf, "=nsx", 4) == 0) {
		DPRINTF("received data is %s action data head.\n", buf);
		return 1;
	} else if(strncmp(buf, "=sx", 3) == 0) {
		DPRINTF("received stream data %s\n", buf);
		return 2;
	} else if(strncmp(buf, "=ll", 3) == 0) {
		DPRINTF("received net data stream%s\n", buf);
		return 3;
	} else if(strncmp(buf, "over", 4) == 0) {
		DPRINTF("received over signal\n");
		return 100;
	} else {
		DPRINTF("other data stream %s\n", buf);
		return 4;
	}
	return -1;
}
void *handler_connetcion(void *arg)
{
	struct data_list_head *head = NULL;
	struct action_data *cur = NULL;
	char buf[BUFLEN] = {0};
	char *p = NULL;
	int new_sock;
	int num_rev = 0;
	int num_count = -1;
	new_sock = (int)arg;
	while(1) {
		memset(buf, 0, BUFLEN);
		if((num_rev = data_rec(new_sock, buf)) < 0) {
			//TODO: error handler
			if(head != NULL) {
				save_action_data(head);
				DPRINTF("data saved number is %d\n", head->data_count - num_count);
				while(head->head != NULL) {
					cur = head->head;
					head->head = cur->next;
					free(cur);
				}
				free(head);
				head = NULL;
			}
			close(new_sock);
			DPRINTF("thread exit %d\n", (int)pthread_self());
			pthread_exit(0);
		}
		switch(judge_data_type(buf, num_rev)) {
			//1:nsx 2:sx 3:ll 100:over -1:other
			case 1:
				p = strchr(buf, '+');
				num_count = atoi(p);
				head = (struct data_list_head *)malloc(sizeof(struct data_list_head));
				head->data_count = num_count;
				head->head = NULL;
				send(new_sock, "ready", 5, 0);
				break;
			case 2:
				if(num_count == -1)
					break;
				cur = decode_data_action(buf);
				if(cur == NULL) {
					DPRINTF("one error data stream\n");
					head->data_count--;
				} else {
					if(head->head == NULL) {
						head->head = cur;
						cur->next = NULL;
					} else {
						cur->next = head->head;
						head->head = cur;
					}
				}
				num_count--;
				send(new_sock, "ok", 2, 0);
				if(num_count == 0) {
					save_action_data(head);
					DPRINTF("data saved number is %d\n", head->data_count - num_count);
					while(head->head != NULL) {
						cur = head->head;
						head->head = cur->next;
						free(cur);
					}
					free(head);
					head = NULL;
					num_count = -1;
				}
				break;
			case 3:
				decode_data_bytes(buf + 3);
				send(new_sock, "ok", 2, 0);
				break;
			case 100:
				if(head != NULL) {
					save_action_data(head);
					DPRINTF("data saved number is %d\n", head->data_count - num_count);
					while(head->head != NULL) {
						cur = head->head;
						head->head = cur->next;
						free(cur);
					}
					free(head);
					head = NULL;
				}
				send(new_sock, "ok", 2, 0);
				close(new_sock);
				DPRINTF("thread exit %d\n", (int)pthread_self());
				pthread_exit(0);
				break;
			case -1:
				//TODO:data formate error handler
				break;
			default:
				//TODO:other data formate handler
				break;
		}
	}
}

int main(int argc,char **argv)
{
	int sockfd, new_fd;
	struct sockaddr_in my_addr;
	struct sockaddr_in des_addr;
	socklen_t sin_size;
	int thread_result;
	char c;
	while ( (c = getopt(argc, argv, "d") ) != -1) {
		switch (c) {
			case 'd':
				Debug = 1;
				break;
		}
	}
	if(Debug != 1) {
		daemonize(argv[0] );
	}
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error while create TCP socket\n");
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(SERVICEPORT);
	my_addr.sin_addr.s_addr = htons(INADDR_ANY);
	bzero(&(my_addr.sin_zero), 8);
	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
		DPRINTF("error while binding socket\n");
		exit(1);
	}
	if(listen(sockfd, 500)) {
		DPRINTF("error while listen socket\n");
		exit(1);
	}
	DPRINTF("Start waiting...\n");
	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		if((new_fd = accept(sockfd, (struct sockaddr *)&des_addr, &sin_size)) == -1) {
			DPRINTF("error in accept connection\n");
			//TODO> add error handle function here
		}
		//Create the thread method
		pthread_t thr_rev;
		pthread_attr_t child_thread_attr;
        	pthread_attr_init(&child_thread_attr);
	        pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);
		//Create new thread for handler connection
		thread_result = pthread_create(&thr_rev, &child_thread_attr, handler_connetcion, (void *)new_fd);
		if(thread_result != 0) {
			DPRINTF("error while create new thread\n");
			//TODO> add error handle function here
		} else {
			DPRINTF("start new thread %d\n", (int)thr_rev);
		}
		
	}
	close(sockfd);
	return 1;
}
