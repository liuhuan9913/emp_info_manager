#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "cli.h"

int main(int argc, const char *argv[])
{
    //创建流式套接字
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        ERR_MSG("socket");
        return -1;
    }

    //绑定IP和端口--->非必须绑定
    // bind，如果不写，系统会自动给客户端分配一个端口号

    //填充服务器的IP和端口，因为客户端必须明确服务器的IP和端口才能连接
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);          //服务器的端口号
    sin.sin_addr.s_addr = inet_addr(IP); //服务器绑定的IP

    //连接服务器
    if (connect(sfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        ERR_MSG("connect");
        return -1;
    }
    printf("连接服务器成功\n");
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("********1登录***********2退出*********\n");
    printf("**************************************\n");
    scanf("%d", &Com.cmdtype);
    
    switch (Com.cmdtype)
    {
    case 1:
        type = log_in(); //登录,向服务器发送登录请求
        if (type == 1)   //管理员操作菜单
        {
            Act_Menu_root();
        }
        if (type == 2) //员工操作菜单
        {
            Act_Menu_user();
        }
        if (type == -2) //登录失败
        {
            return -1; //退出
        }

        

        break;

    case 2:
        return -1; //退出，

    default:
        printf("非法指令,重新输入\n");
        break;
    }

    close(sfd); //关闭套接字
    return 0;
}
