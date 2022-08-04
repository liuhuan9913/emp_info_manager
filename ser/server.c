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

int main(int argc, const char *argv[]) //主线程负责连接
{
	sqlite3 *my_db;			   //数据库初始化
	int ret = sqlite3_open("../my.db", &my_db);
	if(ret != SQLITE_OK)
	{
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(my_db));
		exit(-1);
	}

	int sfd = socket(AF_INET, SOCK_STREAM, 0); //创建流式套接字
	if (sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	int reuse = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) //允许端口快速重用
	{
		ERR_MSG("setsockopt");
		return -1;
	}
	printf("允许端口快速重用\n");

	//填充地址信息结构体
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;			 //必须填AF_INET
	sin.sin_port = htons(PORT);			 //网络字节序的端口号1024~49151
	sin.sin_addr.s_addr = inet_addr(IP); //本机IP，ifconfig查找

	if (bind(sfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) //将IP和端口绑定到套接字上
	{
		ERR_MSG("bind");
		return -1;
	}

	if (listen(sfd, 10) < 0) //将套接字设置为被动监听状态;
	{
		ERR_MSG("listen");
		return -1;
	}
	printf("监听成功\n");

	struct sockaddr_in cin;
	socklen_t addrlen = sizeof(cin);

	int newfd = 0;
	pthread_t tid;
	struct msg cliInfo;

	while (1)
	{
		//主线程负责连接--->accept
		//取出新的文件描述符
		newfd = accept(sfd, (struct sockaddr *)&cin, &addrlen);
		if (newfd < 0)
		{
			ERR_MSG("accept");
			return -1;
		}
		printf("[%s:%d] newfd=%d 连接成功\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);

		cliInfo.newfd = newfd;
		cliInfo.cin = cin;

		//能运行到当前位置，则说明有客户端连接成功
		//则需要创建一个分支线程用于与客户端交互
		if (pthread_create(&tid, NULL, rcv_cli_msg, &cliInfo) != 0)
		{
			ERR_MSG("pthread_create");
			return -1;
		}
	}
	sqlite3_close(my_db); //关闭数据库
	close(sfd);			  //关闭文件描述符
	return 0;
}

void *rcv_cli_msg(void *arg) //线程执行体--->与客户端交互的代码 void* arg = &cliInfo;
{
	pthread_detach(pthread_self()); //分离当前线程，退出后自动回收资源

	struct msg cliInfo = *(struct msg *)arg;
	int newfd = cliInfo.newfd;
	struct sockaddr_in cin = cliInfo.cin;
	ssize_t res = 0;
	int ret = sqlite3_open("../my.db", &cliInfo.db);
	if(ret != SQLITE_OK)
	{
		printf("errcode[%d]  errmsg[%s]\n", ret, sqlite3_errmsg(cliInfo.db));
		exit(-1);
	}
	
	while (1)
	{
		res = recv(newfd, &Com, sizeof(Com), 0); //接收登录信息     recv:1
		if (res < 0)
		{
			ERR_MSG("recv");
			return NULL;
		}
		else if (0 == res)
		{
			fprintf(stderr, "[%s:%d] newfd=%d 客户端关闭\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
			return NULL;
		}

		switch (Com.cmdtype) //判断客户端发来的指令
		{
		case 1:
			log_in(res, newfd, cin, cliInfo.db);
			if (Com.st.id == 1001)
			{
				menu_root(res, newfd, cin, cliInfo.db);
			}else
			{
				menu_user(res, newfd, cin, cliInfo.db);
			}
			
			
			
			break;

		default:
			break;
		}
		
	}
	sqlite3_close(cliInfo.db);
	close(newfd);
	pthread_exit(NULL);
	
}
