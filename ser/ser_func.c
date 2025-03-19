#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdlib.h>
#include "ser.h"

int id_pwd_confirm(void *arg, int column, char **column_text, char **column_name) //回调函数验证账号密码
{
	int newfd = *((int *)arg);
	if (Com.st.id == atoi(column_text[0]))
	{
		if (Com.st.password == atoi(column_text[1])) //用户名密码正确进入
		{
			Com.cmdtype = 0;										   // 0代表成功
			if (send(newfd, &Com.cmdtype, sizeof(Com.cmdtype), 0) < 0) //发送数据到客户端   send:1
			{
				ERR_MSG("send");
				return -1;
			}
		}
		else //密码错误
		{
			Com.cmdtype = 1;										   // 1代表失败
			if (send(newfd, &Com.cmdtype, sizeof(Com.cmdtype), 0) < 0) //发送数据到客户端    send:1
			{
				ERR_MSG("send");
				return -1;
			}
		}
	}
	else //用户名错误
	{
		Com.cmdtype = 1;										   // 1代表失败
		if (send(newfd, &Com.cmdtype, sizeof(Com.cmdtype), 0) < 0) //发送数据到客户端       send:1
		{
			ERR_MSG("send");
			return -1;
		}
	}
	return 0; //必须返回，成功返回0,失败返回非0;
}

int log_in(ssize_t res, int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //登录验证函数
{
	int flag = 0;
	char *errmsg = NULL;
	char buf[128];
	char historybuf[1024];
	char **result = NULL;
	int column;
	int row;
	res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:2
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if (0 == res)
	{
		fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		return -1;
	}
	// switch (Com.usertype)
	switch (Com.usertype)
	{
	case 1:
		sprintf(buf, "select * from emp_info where id = 1001"); //找出管理员的所在记录id = 1001
		if (sqlite3_exec(sql_db, buf, id_pwd_confirm, &newfd, &errmsg) != SQLITE_OK)
		{
			fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
			return -1;
		}
		else
		{
			Com.cmdtype = 1;										   // 1代表失败
			if (send(newfd, &Com.cmdtype, sizeof(Com.cmdtype), 0) < 0) //发送数据到客户端       send:1
			{
				ERR_MSG("send");
				return -1;
			}
			return -1;
		}
		break;

	case 2:
		sprintf(buf, "select * from emp_info where id = %d and password= %d", Com.st.id, Com.st.password); //普通员工的sql语句
		if (sqlite3_exec(sql_db, buf, id_pwd_confirm, &newfd, &errmsg) != SQLITE_OK)
		{
			fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
			return -1;
		}
		else
		{
			Com.cmdtype = 1;										   // 1代表失败
			if (send(newfd, &Com.cmdtype, sizeof(Com.cmdtype), 0) < 0) //发送数据到客户端       send:1
			{
				ERR_MSG("send");
				return -1;
			}
			return -1;
		}
		break;

	default:
		break;
	}

	sprintf(historybuf,"工号:%d登录",Com.st.id);
	history_init(sql_db,historybuf);
	return 0;
}

int menu_root(ssize_t res, int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //管理员菜单函数{}
{
	res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:3
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if (0 == res)
	{
		fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		return -1;
	}

	switch (Com.usertype)
	{
	case 1:
		//添加
		insert_server(newfd, cin, sql_db); //添加函数
		break;
	case 2:
		//删除
		del_server(newfd, cin, sql_db); //删除函数
		break;
	case 3:
		//修改
		update_server_root(newfd, cin, sql_db); //修改函数
		break;
	case 4:
		//查询
		search_server_root(newfd, cin, sql_db); //查询函数
		break;
	case 5:
		//历史
		history_server(newfd,cin,sql_db); //历史函数
		break;
	case 6:
		//退出
		break;
	default:
		break;
	}
}

int menu_user(ssize_t res, int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //用户菜单函数
{
	res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:3
	if (res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if (0 == res)
	{
		fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		return -1;
	}

	switch (Com.usertype)
	{
	case 1:
		//查询
		search_server_user(newfd,cin,sql_db); //查询函数
		break;
	case 2:
		//修改
		update_server_user(newfd, cin, sql_db); //修改函数用户
		break;
	case 3:
		//退出
		break;
	default:
		break;
	}
	return 0;
}

void insert_server(int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //添加函数
{
	int res;
	char buf[1024];
	char historybuf[1024];
	char *errmsg = NULL;
	res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:4
	if (res < 0)
	{
		ERR_MSG("recv");
		return;
	}
	else if (0 == res)
	{
		fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		return;
	}

	sprintf(buf, "insert into emp_info values(%d, %d, \"%s\", %d, %d, \"%s\", %g)", Com.st.id,
			Com.st.password, Com.st.name, Com.st.QQ, Com.st.age, Com.st.addr, Com.st.salary);
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	Com.cmdtype = 0;						   // 0代表成功
	if (send(newfd, &Com, sizeof(Com), 0) < 0) //发送数据到客户端       send:1
	{
		ERR_MSG("send");
		return;
	}

	sprintf(historybuf,"工号:%d被增加",Com.st.id);
	history_init(sql_db,historybuf);
	
	return;
}

void del_server(int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //删除函数
{
	int res;
	char buf[1024];
	char historybuf[1024];
	char *errmsg = NULL;
	res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:4
	if (res < 0)
	{
		ERR_MSG("recv");
		return;
	}
	else if (0 == res)
	{
		fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		return;
	}

	sprintf(buf, "DELETE FROM emp_info WHERE id = %d", Com.st.id);
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	Com.cmdtype = 0;						   // 0代表成功
	if (send(newfd, &Com, sizeof(Com), 0) < 0) //发送数据到客户端       send:1
	{
		ERR_MSG("send");
		return;
	}
	sprintf(historybuf,"工号:%d被删除",Com.st.id);
	history_init(sql_db,historybuf);
	return;
}

void update_server_root(int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //修改函数管理员
{
	int res;
	char buf[1024];
	char historybuf[1024];
	char *errmsg = NULL;
	res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:4
	if (res < 0)
	{
		ERR_MSG("recv");
		return;
	}
	else if (0 == res)
	{
		fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET password = %d where id = %d", Com.st.password, Com.st.id); // password
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET name = \"%s\" where id = %d", Com.st.name, Com.st.id); // name
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET QQ = %d where id = %d", Com.st.QQ, Com.st.id); // QQ
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET age = %d where id = %d", Com.st.age, Com.st.id); // age
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}
	sprintf(buf, "UPDATE emp_info SET addr = \"%s\" where id = %d", Com.st.addr, Com.st.id); // addr
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET salay = %g where id = %d", Com.st.salary, Com.st.id); // salary
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	Com.cmdtype = 0;						   // 0代表成功
	if (send(newfd, &Com, sizeof(Com), 0) < 0) //发送数据到客户端       send:1
	{
		ERR_MSG("send");
		return;
	}
	sprintf(historybuf,"工号:%d被修改",Com.st.id);
	history_init(sql_db,historybuf);
	return;
}

void search_server_root(int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //查询函数管理员
{
	char buf[1024];
	char historybuf[1024];
	char *errmsg = NULL;
	char **result;
	result = NULL;
	int row,column;
	int i;

	sprintf(buf, "select * from emp_info"); //查找所有
	if (sqlite3_get_table(sql_db, buf, &result, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}
	for(i = 0;i < column; i++)
	{
		printf("%-8s",result[i]);
	}
	
	//打印记录表
	//i = 0;
	int index = i;
	for(i = 0;i < row; i++)
	{
		printf("%s,    %s,    %s,    %s,    %s,    %s,    %s,    ;",result[index+column-7],result[index+column-6],result[index+column-5],\
				result[index+column-4],result[index+column-3],result[index+column-2],result[index+column-1]);
		sprintf(Com.buf,"%s\t%s\t%s\t%s\t%s\t%s\t%s",result[index+column-7],result[index+column-6],result[index+column-5],\
				result[index+column-4],result[index+column-3],result[index+column-2],result[index+column-1]);
		
		if (send(newfd, &Com, sizeof(Com), 0) < 0) //发送数据到客户端       send:1
		{
			ERR_MSG("send");
			return;
		}
		index += column;
	}
	strcpy(Com.buf,"over*");
	send(newfd,&Com,sizeof(Com),0);

	sqlite3_free_table(result);

	sprintf(historybuf,"用户:%d进行了查询",Com.st.id);
	history_init(sql_db,historybuf);
	return;
}

void history_server(int newfd,struct sockaddr_in cin,sqlite3 *sql_db) //历史函数
{
	char buf[1024];
	char historybuf[1024];
	char *errmsg = NULL;
	char **result;
	result = NULL;
	int row,column;
	int i;

	sprintf(buf, "select * from historyinfo"); //查找所有
	if (sqlite3_get_table(sql_db, buf, &result, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}
	for(i = 0;i < column; i++)
	{
		printf("%-8s",result[i]);
	}
	
	//打印记录表
	//i = 0;
	int index = i;
	for(i = 0;i < row; i++)
	{
		printf("%s,    %s,    %s",result[index+column-3],result[index+column-2],result[index+column-1]);
		sprintf(Com.buf,"%s\t%s\t%s",result[index+column-3],result[index+column-2],result[index+column-1]);
		
		if (send(newfd, &Com, sizeof(Com), 0) < 0) //发送数据到客户端       send:1
		{
			ERR_MSG("send");
			return;
		}
		index += column;
	}
	strcpy(Com.buf,"over*");
	send(newfd,&Com,sizeof(Com),0);
	sqlite3_free_table(result);
	return;
}

void update_server_user(int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //修改函数用户
{
	printf("Com.st.id = %d\n", Com.st.id);
	printf("__LINE__ = %d\n", __LINE__);
	int res;
	char buf[1024];
	char historybuf[1024];
	char *errmsg = NULL;
	res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:4
	if (res < 0)
	{
		ERR_MSG("recv");
		return;
	}
	else if (0 == res)
	{
		fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET password = %d where id = %d", Com.st.password, Com.st.id); // password
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET name = \"%s\" where id = %d", Com.st.name, Com.st.id); // name
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET QQ = %d where id = %d", Com.st.QQ, Com.st.id); // QQ
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET age = %d where id = %d", Com.st.age, Com.st.id); // age
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}
	sprintf(buf, "UPDATE emp_info SET addr = \"%s\" where id = %d", Com.st.addr, Com.st.id); // addr
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	sprintf(buf, "UPDATE emp_info SET salay = %g where id = %d", Com.st.salary, Com.st.id); // salary
	if (sqlite3_exec(sql_db, buf, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}

	Com.cmdtype = 0;						   // 0代表成功
	if (send(newfd, &Com, sizeof(Com), 0) < 0) //发送数据到客户端       send:1
	{
		ERR_MSG("send");
		return;
	}

	sprintf(historybuf,"工号:%d被修改",Com.st.id);
	history_init(sql_db,historybuf);
	return;
}

void search_server_user(int newfd, struct sockaddr_in cin, sqlite3 *sql_db) //查询函数用户
{
	char buf[1024];
	char historybuf[1024];
	char *errmsg = NULL;
	char **result;
	result = NULL;
	int row,column;
	int i;

	sprintf(buf, "select * from emp_info where id = %d",Com.st.id); //查找所有
	if (sqlite3_get_table(sql_db, buf, &result, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return;
	}
	for(i = 0;i < column; i++)
	{
		printf("%-8s",result[i]);
	}
	
	//打印记录表
	//i = 0;
	int index = i;
	for(i = 0;i < row; i++)
	{
		printf("%s,    %s,    %s,    %s,    %s,    %s,    %s,    ;",result[index+column-7],result[index+column-6],result[index+column-5],\
				result[index+column-4],result[index+column-3],result[index+column-2],result[index+column-1]);
		sprintf(Com.buf,"%s\t%s\t%s\t%s\t%s\t%s\t%s",result[index+column-7],result[index+column-6],result[index+column-5],\
				result[index+column-4],result[index+column-3],result[index+column-2],result[index+column-1]);
		if (send(newfd, &Com, sizeof(Com), 0) < 0) //发送数据到客户端       send:1
		{
			ERR_MSG("send");
			return;
		}
		index += column;
	}
	strcpy(Com.buf,"over*");
	send(newfd,&Com,sizeof(Com),0);
	sqlite3_free_table(result);

	sprintf(historybuf,"用户:%d进行了查询",Com.st.id);
	history_init(sql_db,historybuf);
	return;
}

void get_system_time(char* timedata)//获取时间
{
	time_t t;
	struct tm *tp;

	time(&t);
	tp = localtime(&t);
	sprintf(timedata,"%d-%d-%d %d:%d:%d",tp->tm_year+1900,tp->tm_mon+1,\
			tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);
	return ;
}

void history_init(sqlite3 *sql_db,char *buf)//历史记录表
{
	//获取当前时间--封装sql命令---将buf用户的操作记录插入到历史记录的表当中
	int nrow,ncolumn;
	char *errmsg, **resultp;

	char sqlhistory[128] = {0};
	char timedata[128] = {0};

	get_system_time(timedata);

	sprintf(sqlhistory,"insert into historyinfo values ('%s','%d','%s');",timedata,Com.st.id,buf);
	if(sqlite3_exec(sql_db,sqlhistory,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		printf("插入历史记录表失败\n");
	}else{
		printf("插入历史记录表成功\n");
	}
}


