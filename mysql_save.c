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


void save_action_data(struct action_data *data)
{
	MYSQL vmysql;
	MYSQL *mysql = &vmysql;
	char sql_str[128] = {'\0'};
	char *sql_fmt = "insert into BankAction values(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\")";
	if(conn_mysql_init(mysql, "BankData")){
		DPRINTF("conn mysql error\n");
		return;
	}
	snprintf(sql_str, sizeof(sql_str), sql_fmt, data->server_code, data->mobile_code, data->login_time, data->logout_time, data->cur_time);
	DPRINTF("sql string is %s\n", sql_str);
	if(mysql_query(mysql, sql_str)) {
		DPRINTF("error to query string\n");
		return;
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
	DPRINTF("sql string is %s\n", sql_str);
	if(mysql_query(mysql, sql_str)) {
		DPRINTF("error to query string\n");
		return;
	}
	mysql_close(mysql);
	return;
}
