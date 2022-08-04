#ifndef __SER_H__
#define __SER_H__

#define ERR_MSG(msg)                          \
	do                                        \
	{                                         \
		fprintf(stderr, "__%d__:", __LINE__); \
		perror(msg);                          \
	} while (0)

#define PORT 8888			 // 1024~49151
#define IP "192.168.250.100" //本机IP，ifconfig查找

struct info
{
	int id;			//员工工号
	int password;	//员工密码
	char name[36];	//员工姓名
	int QQ;			//员工QQ
	int age;		//员工年龄
	char addr[128]; //员工家庭住址
	float salary;	//员工工资
};					//员工信息结构体

struct Communication
{
	int usertype;	//用户类型
	int cmdtype;	//通信指令类型
	char buf[1024];	//通信的消息
	struct info st; //员工信息
};					//通信结构体

struct Communication Com;


//需要传入到分支线程的数据类型;
struct msg
{
	int newfd;
	struct sockaddr_in cin;
	sqlite3 *db;
};

//sqlite3 *proc_init();//数据库初始化函数声明
void *rcv_cli_msg(void *arg); // void* arg = &cliInfo; 线程声明
int log_in(ssize_t res, int newfd, struct sockaddr_in cin, sqlite3 *sql_db);//登录函数声明
int id_pwd_confirm(void *arg, int column, char **column_text, char **column_name);//用户名密码确认函数声明

int menu_root(ssize_t res, int newfd, struct sockaddr_in cin, sqlite3 *sql_db);//管理员菜单函数声明
int menu_user(ssize_t res, int newfd, struct sockaddr_in cin, sqlite3 *sql_db);//员工菜单函数声明
void insert_server(int newfd,struct sockaddr_in cin,sqlite3 *sql_db);//添加函数声明
void del_server(int newfd,struct sockaddr_in cin,sqlite3 *sql_db);//删除函数声明
void update_server_root(int newfd,struct sockaddr_in cin,sqlite3 *sql_db);//修改函数管理员声明
void search_server_root(int newfd,struct sockaddr_in cin,sqlite3 *sql_db);//查询函数声明管理员
void search_server_user(int newfd, struct sockaddr_in cin, sqlite3 *sql_db);//查询函数声明用户
void history_server(int newfd,struct sockaddr_in cin,sqlite3 *sql_db);//历史函数声明
void update_server_user(int newfd,struct sockaddr_in cin,sqlite3 *sql_db);//修改函数用户声明
void get_system_time(char* timedata);//获取时间
void history_init(sqlite3 *sql_db,char *buf);//历史记录表


#endif 