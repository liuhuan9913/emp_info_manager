#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "cli.h"

int log_in() //登录函数
{
    int res;
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("****1管理员登录***2员工登录***3退出****\n");
    printf("**************************************\n");
    scanf("%d", &Com.usertype);
    if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:1
    {
        ERR_MSG("send");
        return -1;
    }
    if (Com.usertype == 3) //判断用户是否需要退出
    {
        return -1;
    }

    if (Com.usertype == 1) //管理员
    {
        printf("请输入用户名(工号)>>>");      
        scanf("%d", &Com.st.id);
        printf("请输入密码>>>");       
        scanf("%d", &Com.st.password);
        if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:2
        {
            ERR_MSG("send");
            return -1;
        }
        res = recv(sfd, &Com.cmdtype, sizeof(Com.cmdtype), 0); //接收服务器发来的确认    recv:1
        if (res < 0)
        {
            ERR_MSG("recv");
            return -1;
        }
        else if (0 == res)
        {
            printf("服务器关闭\n");
            return -1;
        }
        if (Com.cmdtype == 0) // 0代表成功
        {
            printf("登录成功\n");
        }
        else if (Com.cmdtype == 1) // 1代表失败
        {
            printf("用户名或密码错误\n");
            return -2; //-2代表登录失败
        }
    }

    if (Com.usertype == 2) //员工
    {
        printf("请输入用户名(工号)>>>");
        scanf("%d", &Com.st.id);
        printf("请输入密码>>>");
        scanf("%d", &Com.st.password);
        if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:2
        {
            ERR_MSG("send");
            return -1;
        }
        res = recv(sfd, &Com.cmdtype, sizeof(Com.cmdtype), 0); //接收服务器发来的确认    recv:1
        if (res < 0)
        {
            ERR_MSG("recv");
            return -1;
        }
        else if (0 == res)
        {
            printf("服务器关闭\n");
            return -1;
        }
        if (Com.cmdtype == 0) // 0代表成功
        {
            printf("登录成功\n");
        }
        else if (Com.cmdtype == 1) // 1代表失败
        {
            printf("用户名或密码错误\n");
            return -2; //-2代表登录失败
        }
    }

    if (Com.usertype != 1 && Com.usertype != 2 && Com.usertype != 3)
    {
        printf("未识别的命令，退出程序\n");
        return -1;
    }

    return Com.usertype;
}

int Act_Menu_root() //操作菜单函数管理员
{
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("**************管理员界面**************\n");
    printf("********1添加***********2删除*********\n");
    printf("********3修改***********4查询*********\n");
    printf("********5历史***********6退出*********\n");
    printf("**************************************\n");
    printf("请输入你的选择>>>");
    scanf("%d", &Com.usertype);
    if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:3
    {
        ERR_MSG("send");
        return -1;
    }

    switch (Com.usertype)
    {
    case 1:
        //添加
        insert_client(); //添加函数
        break;
    case 2:
        //删除
        del_client(); //删除函数
        break;
    case 3:
        //修改
        update_client_root(); //修改函数
        break;
    case 4:
        //查询
        search_client(); //查询函数
        break;
    case 5:
        //历史
        history_client(); //历史函数
        break;
    case 6:
        //退出
        break;
    default:
        break;
    }
    return 0;
}

int Act_Menu_user() //操作菜单函数员工
{
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("***************员工界面***************\n");
    printf("********1查询***********3退出*********\n");
    printf("****************2修改*****************\n");
    printf("**************************************\n");
    printf("请输入你的选择>>>");
    scanf("%d", &Com.usertype);
    if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:3
    {
        ERR_MSG("send");
        return -1;
    }

    switch (Com.usertype)
    {
    case 1:
        //查询
        search_client(); //查询函数
        break;
    case 2:
        //修改
        update_client_user(); //修改函数用户
        break;
    case 3:
        //退出
        break;
    default:
        break;
    }
    return 0;
}

void insert_client() //添加函数
{
    
    int res;
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("*************进入添加页面*************\n");
    printf("*************************************\n");
    /*
    struct info
    {
        int id;         //员工工号
        int password;   //员工密码
        char name[36];  //员工姓名
        int QQ;         //员工QQ
        int age;        //员工年龄
        char addr[128]; //员工家庭住址
        float salary;   //员工工资
    };
    */
    printf("请输入需要添加的员工的用户名(工号)>>>");
    scanf("%d", &Com.st.id);
    printf("请输入需要添加的员工的密码>>>");
    scanf("%d", &Com.st.password);
    printf("请输入需要添加的员工的姓名>>>");
    scanf("%s", &Com.st.name);
    printf("请输入需要添加的员工的QQ>>>");
    scanf("%d", &Com.st.QQ);
    printf("请输入需要添加的员工的年龄>>>");
    scanf("%d", &Com.st.age);
    printf("请输入需要添加的员工的家庭住址>>>");
    scanf("%s", &Com.st.addr);
    printf("请输入需要添加的员工的工资>>>");
    scanf("%g", &Com.st.salary);

    if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:4
    {
        ERR_MSG("send");
        return;
    }

    res = recv(sfd, &Com, sizeof(Com), 0); //接收服务器发来的确认    recv:2
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    else if (0 == res)
    {
        printf("服务器关闭\n");
        return;
    }

    if (Com.cmdtype == 0) // 0代表成功
    {
        printf("添加成功\n");
        return;
    }
    else if (Com.cmdtype == 1) // 1代表失败
    {
        printf("添加失败\n");
        return;
    }

    
}

void del_client() //删除函数
{
    int res;
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("*************进入删除页面*************\n");
    printf("*************************************\n");
    printf("请输入需要删除的员工的用户名(工号)>>>");
    scanf("%d", &Com.st.id);
    
    if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:4
    {
        ERR_MSG("send");
        return;
    }

    res = recv(sfd, &Com, sizeof(Com), 0); //接收服务器发来的确认    recv:2
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    else if (0 == res)
    {
        printf("服务器关闭\n");
        return;
    }

    if (Com.cmdtype == 0) // 0代表成功
    {
        printf("删除成功\n");
        return;
    }
    else if (Com.cmdtype == 1) // 1代表失败
    {
        printf("删除失败\n");
        return;
    }

}

void update_client_root() //修改函数管理员
{
    int res;
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("*************进入修改页面*************\n");
    printf("*************************************\n");
    printf("请输入需要修改的员工的用户名(工号)>>>");
    scanf("%d", &Com.st.id);
    
    printf("请输入需要修改该员工的密码>>>");
    scanf("%d", &Com.st.password);
    printf("请输入需要修改该员工的姓名>>>");
    scanf("%s", &Com.st.name);
    printf("请输入需要修改该员工的QQ>>>");
    scanf("%d", &Com.st.QQ);
    printf("请输入需要修改该员工的年龄>>>");
    scanf("%d", &Com.st.age);
    printf("请输入需要修改该员工的家庭住址>>>");
    scanf("%s", &Com.st.addr);
    printf("请输入需要修改该员工的工资>>>");
    scanf("%g", &Com.st.salary);


    if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:4
    {
        ERR_MSG("send");
        return;
    }

    res = recv(sfd, &Com, sizeof(Com), 0); //接收服务器发来的确认    recv:2
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    else if (0 == res)
    {
        printf("服务器关闭\n");
        return;
    }

    if (Com.cmdtype == 0) // 0代表成功
    {
        printf("修改成功\n");
        return;
    }
    else if (Com.cmdtype == 1) // 1代表失败
    {
        printf("修改失败\n");
        return;
    }
}

void search_client() //查询函数
{
    char buff[4096];
    int i;
    int res;
    res = recv(sfd, &Com, sizeof(Com), 0); //接收服务器发来的确认    recv:2
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    else if (0 == res)
    {
        printf("服务器关闭\n");
        return;
    }
    
    printf("工号\t密码\t姓名\tQQ\t年龄\t家庭住址\t工资\n");
	while (1)
	{	
		//循环接受服务器发送的用户数据
		recv(sfd, &Com, sizeof(Com), 0);
		if(strncmp(Com.buf , "over*",5) ==0)
			break;
		printf("%s\n",Com.buf);
        
	}
         

}

void history_client() //历史函数
{
    while(1){
		recv(sfd, &Com, sizeof(Com), 0);
		if(strncmp(Com.buf ,"over*",5) == 0)
			break;	
		printf("%s",Com.buf);
        printf("\n");
	}
	printf("admin查询历史记录结束!\n");
}

void update_client_user() //修改函数用户
{
    int res;
    printf("*************************************\n");
    printf("*************员工管理系统*************\n");
    printf("*************进入修改页面*************\n");
    printf("*************************************\n");
    
    printf("请输入需要修改的密码>>>");
    scanf("%d", &Com.st.password);
    printf("请输入需要修改的姓名>>>");
    scanf("%s", &Com.st.name);
    printf("请输入需要修改的QQ>>>");
    scanf("%d", &Com.st.QQ);
    printf("请输入需要修改的年龄>>>");
    scanf("%d", &Com.st.age);
    printf("请输入需要修改的家庭住址>>>");
    scanf("%s", &Com.st.addr);
    printf("请输入需要修改的工资>>>");
    scanf("%g", &Com.st.salary);


    if (send(sfd, &Com, sizeof(Com), 0) < 0) //向服务器发送数据    send:4
    {
        ERR_MSG("send");
        return;
    }

    res = recv(sfd, &Com, sizeof(Com), 0); //接收服务器发来的确认    recv:2
    if (res < 0)
    {
        ERR_MSG("recv");
        return;
    }
    else if (0 == res)
    {
        printf("服务器关闭\n");
        return;
    }

    if (Com.cmdtype == 0) // 0代表成功
    {
        printf("修改成功\n");
        return;
    }
    else if (Com.cmdtype == 1) // 1代表失败
    {
        printf("修改失败\n");
        return;
    }
}

