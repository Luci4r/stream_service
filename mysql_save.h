#ifndef __MYSQL_CONN_H__
#define __MYSQL_CONN_H__
#include"global.h"
#include <mysql/mysql.h>
#define HOST "127.0.0.1"
#define USER "root"
#define PASSWD "111"

void save_action_data(struct action_data *data);
void save_stream_data(struct stream_data *data);
#endif /*__MYSQL_CONN_H__*/
