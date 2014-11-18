#include "mysql_save.h"
int conn_mysql_init(MYSQL *mysql, char *dbname)
{
	char value = 1;
	mysql_init(mysql);
	if (!mysql_real_connect(mysql, HOST, USER, PASSWD, dbname, 0, NULL, 0) ) {
		DPRINTF("failed to connect to mysql error: %s. exit\n", mysql_error(mysql));
		return -1;
	}
	mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *)&value);
	if (mysql_set_character_set(mysql, "utf8") ) {
		mysql_close(mysql);
		return -1;
	}
	return 0;
}


void save_action_data(struct data_list_head *data)
{
	MYSQL vmysql;
	MYSQL *mysql = &vmysql;
	struct action_data *cur;
	char sql_str[128] = {'\0'};
	int data_count = 0;
	char *sql_fmt = "insert into BankAction values(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\")";
	if(conn_mysql_init(mysql, "BankData")){
		DPRINTF("conn mysql error\n");
		return;
	}
	cur = data->head;
	data_count = data->data_count;
	while(cur != NULL && data_count > 0) {
		snprintf(sql_str, sizeof(sql_str), sql_fmt, cur->server_code, cur->mobile_code, cur->login_time, cur->logout_time, cur->cur_time);
#ifdef DEBUG
		DPRINTF("sql string is %s\n", sql_str);
#endif
		if(mysql_query(mysql, sql_str)) {
			DPRINTF("error to query string\n");
			continue;
		}
		cur = cur->next;
		data_count--;
	}
	if(cur != NULL || data_count != 0) {
		//TODO: data no saved handler
		DPRINTF("some data is not saved\n");
	}
	mysql_close(mysql);
	return;
}

void save_stream_data(struct stream_data *data)
{
	MYSQL vmysql;
	MYSQL *mysql = &vmysql;
	char sql_str[100] = {'\0'};
	char *sql_fmt = "insert into BankStream values(\"%s\",\"%s\",\"%s\",\"%s\")";
	if(conn_mysql_init(mysql, "BankData")){
		DPRINTF("conn mysql error\n");
		return;
	}
	snprintf(sql_str, sizeof(sql_str), sql_fmt, data->server_code, data->RX_bytes, data->TX_bytes, data->cur_time);
#ifdef DEBUG
	DPRINTF("sql string is %s\n", sql_str);
#endif
	if(mysql_query(mysql, sql_str)) {
		DPRINTF("error to query string\n");
		return;
	}
	mysql_close(mysql);
	return;
}
