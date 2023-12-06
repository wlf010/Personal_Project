#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <pthread.h>

#define STAFF_DATABASE 	 "staff_manage_system.db"

#define ERR_MSG(msg) do{\
	printf("__%d__ %s\n", __LINE__, __func__);\
	perror(msg);\
}while(0)


#define USER_LOGIN 		0x00000000  // login	登陆    0x00000001
#define USER_MODIFY 	0x00000001  // user-modification  修改
#define USER_QUERY 		0x00000002  // user-query   查询

#define ADMIN_LOGIN 	0x10000000  // login	登陆    0x00000001
#define ADMIN_MODIFY 	0x10000001 // admin_modification
#define ADMIN_ADDUSER 	0x10000002 // admin_adduser
#define ADMIN_DELUSER 	0x10000004 // admin_deluser
#define ADMIN_QUERY 	0x10000008  //hitory_query
#define ADMIN_HISTORY 	0x10000010  //hitory_query

#define PORT 	8888
#define IP 		"192.168.1.111" 

#define QUIT 			0x11111111

#define ADMIN 0	//管理员
#define USER  1	//用户

#define NAMELEN 16
#define DATALEN 64

/*员工基本信息*/
typedef struct staff_info{
	int  no; 			//员工编号
	int  usertype;  	//ADMIN 1	USER 2	 
	char name[NAMELEN];	//姓名
	char passwd[8]; 	//密码
	int  age; 			// 年龄
	char phone[NAMELEN];//电话
	char addr[DATALEN]; // 地址
	char work[DATALEN]; //职位
	char date[DATALEN];	//入职年月
	int level;			// 等级
	double salary ;		// 工资
	
}staff_info_t;

/*定义双方通信的结构体信息*/
typedef struct {
	int  msgtype;     //请求的消息类型
	int  usertype;    //ADMIN 1	USER 2	   
	char username[NAMELEN];  //姓名
	char passwd[8];			 //登陆密码
	char recvmsg[DATALEN];   //通信的消息
	int  flags;      //标志位
	char time[256];
	staff_info_t info;      //员工信息
}MSG;


int user_menu(int sockfd,MSG *msg);
int admin_or_user_login(int sockfd,MSG *msg);
void do_admin_user_query(int sockfd,MSG *msg);
void do_user_admin_modification(int sockfd,MSG *msg);
int updata_user_mod_int(int socked,MSG *msg);
int do_user_mod_no(int sockfd,MSG *msg);
int do_user_mod_usertype(int sockfd,MSG *msg);
int do_user_mod_age(int sockfd,MSG *msg);
int do_user_mod_level(int sockfd,MSG *msg);

int updata_user_mod_char(int sockfd,MSG *msg);
int do_user_mod_name(int sockfd,MSG *msg);
int do_user_mod_passwd(int sockfd,MSG *msg);
int do_user_mod_phone(int sockfd,MSG *msg);
int do_user_mod_addr(int sockfd,MSG *msg);
int do_user_mod_work(int sockfd,MSG *msg);
int do_user_mod_date(int sockfd,MSG *msg);
int do_user_mod_salary(int sockfd,MSG *msg);

void clear_all();
void do_admin_query_all(int sockfd,MSG *msg);
void do_admin_adduser(int sockfd,MSG *msg);
void do_admin_deluser(int sockfd,MSG *msg);
void do_admin_modification(int sockfd,MSG *msg);

int  do_login(int sockfd);
void admin_menu(int sockfd,MSG *msg);
void user_menu_a();
void do_user_modification_a();
void admin_menu_a();
void do_admin_modification_a();
void do_admin_history(int sockfd,MSG *msg);
void printf_all(MSG *msg);

#endif
