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
	double salary ;		// 工
	
}staff_info_t;

/*定义双方通信的结构体信息*/
typedef struct {
	int  msgtype;     			//请求的消息类型
	int  usertype;    			//ADMIN 1	USER 2	   
	char username[NAMELEN];  //姓名
	char passwd[8];			 //登陆密码
	char recvmsg[DATALEN];   //通信的消息
	int  flags;      //标志位
	char time[256];
	staff_info_t info;      //员工信息
}MSG;

typedef struct{
    int newfd;
    struct sockaddr_in cin;
}allmsg;

sqlite3 *db;

int process_client_request(int newfd,MSG *msg);

int process_user_or_admin_login_request(int newfd,MSG *msg);
int process_user_query_request(int acceptfd,MSG *msg,char sql[]);
int process_user_modify_request(int acceptfd,MSG *msg);

int mod_user_all(int acceptfd,char sql[],MSG *msg,char buf[]);

int process_admin_query_request(int acceptfd,MSG *msg);
int process_admin_adduser_request(int acceptfd,MSG *msg);
int process_admin_deluser_request(int acceptfd,MSG *msg);
int process_admin_history_request(int acceptfd,MSG *msg);
int updata_hidtory(int acceptfd,MSG * msg,char work[]);

void printf_all(MSG *msg);

#endif