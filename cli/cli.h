#ifndef __CLI_H__
#define __CLI_H__

#define ERR_MSG(msg)                          \
    do                                        \
    {                                         \
        fprintf(stderr, "__%d__:", __LINE__); \
        perror(msg);                          \
    } while (0)

#define PORT 8888            //端口号
#define IP "192.168.250.100" //服务器IP

struct info
{
    int id;         //员工工号
    int password;   //员工密码
    char name[36];  //员工姓名
    int QQ;         //员工QQ
    int age;        //员工年龄
    char addr[128]; //员工家庭住址
    float salary;   //员工工资
};                  //员工信息结构体

struct Communication
{
    int usertype;   //用户类型
    int cmdtype;    //通信指令类型
    char buf[1024];   //通信的消息
    struct info st; //员工信息
};                  //通信结构体



struct Communication Com; //通信结构体初始化
int sfd;                  //文件描述符初始化
int type;//登录用户标识 1管理员 2员工

int log_in(); //登录函数声明
int Act_Menu_root();//管理员操作菜单函数声明
int Act_Menu_user();//员工操作菜单函数声明
void insert_client();//添加函数声明
void del_client();//删除函数声明
void update_client_root();//修改函数管理员声明
void search_client();//查询函数声明
void history_client();//历史函数声明
void update_client_user();//修改函数用户声明

#endif