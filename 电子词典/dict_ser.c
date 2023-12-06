#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>

#define ERR_LOG(msg) do{\
	printf("%d %s \n", __LINE__, __func__);\
	perror(msg);\
}while(0)   //错误信息

#define PORT 8765  //自定义端口信息
#define IP "192.168.1.106" //本机IP
#define N 256  //sql数组存储
#define M 128  //通讯文本存储


typedef struct   //客户端信息
{
	int fd ;
	struct sockaddr_in cin;
	sqlite3* user_db;
	sqlite3* dict_db;
}_cin;

struct msg  // 通讯结构体
{
	int type;
	char name[20];
	char text[M];
};
//保存历史记录
int history_send(int cntfd,struct msg recv_msg,char *word,char **pres, sqlite3* user_db);
int init_sqlite(sqlite3** , sqlite3** ); //数据库初始化
int do_dict_txt(sqlite3* );  //导入电子词典
int init_socket(int* );   //网络初始化
void* handle_cli_msg(void* );   //分支线程处理
int do_register(int , struct msg , sqlite3* );  //注册
int do_login(int , struct msg , sqlite3* );  //登录
int do_select(int , struct msg , sqlite3* , sqlite3* );  //查单词
int do_history(int , struct msg , sqlite3*);  //查历史记录
int do_quit(int , struct msg ,sqlite3* );  //退出


int main(int argc, const char *argv[])
{


	//数据库初始化
	sqlite3* dict_db = NULL;
	sqlite3* user_db = NULL;
	if(init_sqlite(&dict_db, &user_db)<0)
	{
		return -1;
	}

	//网络初始化
	int sfd = 0;
	if(init_socket(&sfd) < 0)
	{
		return -1;
	}
	//多线程,主线程连接,分支线程通信
	int newfd = 0;
	struct sockaddr_in cin;
	socklen_t clen = sizeof(cin);
	while(1)
	{
		newfd = accept(sfd, (void*)&cin, &clen);
		if(newfd < 0)
		{
			ERR_LOG("accept");
			return -1;
		}
		printf("----[%s:%d]连接成功----\n", (char*)inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));

		//连接成功,创建分支线程
		_cin cli_msg = {newfd, cin, user_db, dict_db};
		pthread_t tid;
		if(pthread_create(&tid, NULL, handle_cli_msg, &cli_msg) !=0)
		{
			ERR_LOG("pthread_create");
			return -1;
		}

	}
	
	return 0;
}

//服务器与客户端交互的函数
void* handle_cli_msg(void* arg)
{
	//线程分离
	pthread_detach(pthread_self());

	_cin cli_msg = *(_cin*)arg;
	int cntfd = cli_msg.fd;
	struct sockaddr_in cin = cli_msg.cin;
	sqlite3* user_db = cli_msg.user_db;
	sqlite3* dict_db = cli_msg.dict_db; 

	int recv_len;
	struct msg recv_msg;

	while(1)
	{
		recv_len = recv(cntfd, &recv_msg, sizeof(recv_msg), 0);
		if(recv_len < 0)
		{
			ERR_LOG("recv_len");
			break;
		}
		else if(0 == recv_len)
		{
			printf("----[%s:%d]断开连接----\n", (char*)inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
			break;
		}

		int type = recv_msg.type;

		switch(type)
		{
			case 'R':  //注册
				do_register(cntfd, recv_msg, user_db);
				break;
			case 'L': //登录
				do_login(cntfd, recv_msg, user_db);
				break;  
			case 'S':  //查单词
				do_select(cntfd, recv_msg, user_db, dict_db);
				break;
			case 'H':  //查历史记录
				do_history(cntfd, recv_msg, user_db);
				break;
			case 'Q':  //退出客户端
				printf("----[%s:%d]断开连接----\n", (char*)inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
				close(cntfd);
				pthread_exit(NULL);
				return 0;
			case 'q':   //退出用户
				do_quit(cntfd, recv_msg, user_db);
				break;
		}
	}

	close(cntfd);
	pthread_exit(NULL);
}
//客户端退出
int do_quit(int cntfd, struct msg recv_msg, sqlite3* user_db)
{
	//客户端退出,修改用户登录状态
	char* errmsg = NULL;
	char sql[N] = "";
	sprintf(sql, "update user set stage = 0 where name=\"%s\";", recv_msg.name);
	char** pres = NULL;
	int row, column;
	if(sqlite3_get_table(user_db, sql, &pres, &row, &column,  &errmsg) != 0)
	{
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}

	printf("%s用户退出\n",recv_msg.name);
	return 0;
}

//历史记录 
int do_history(int cntfd, struct msg recv_msg, sqlite3*user_db)
{
	char* errmsg = NULL;
	char sql[N] = "";

	//查询格式:单词 意思 时间
	sprintf(sql, "select words,means,time  from history where name=\"%s\";", recv_msg.name);

	char** pres = NULL;
	int row, column;
	if(sqlite3_get_table(user_db, sql, &pres, &row, &column,  &errmsg) != 0)
	{
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}
	//如果没有找到
	if(0 == row)
	{
		strcpy(recv_msg.text, "not find");
		if(send(cntfd, &recv_msg, sizeof(recv_msg), 0) <0)
		{
			ERR_LOG("send");
			return -1;
		}

	}
	else	//如果找到了发送单词相应的意思
	{
		bzero(recv_msg.text, sizeof(recv_msg.text));
		int i = 0, j=0; 	
		for(i=1; i<row+1; i++)
		{//循环发送找到的结果
			j=i*column;
			if(i==row)
			{//如果为最后一个,同时发送结束协议
				sprintf(recv_msg.text, "%-10s %-20s %s", pres[j], pres[j+1], pres[j+2]);
				recv_msg.type='O';
				if(send(cntfd, &recv_msg, sizeof(recv_msg), 0) <0)
				{
					ERR_LOG("send");
					return -1;
				}
			}
			else
			{
				sprintf(recv_msg.text, "%-10s %-20s %s", pres[j], pres[j+1], pres[j+2]);
				if(send(cntfd, &recv_msg, sizeof(recv_msg), 0) <0)
				{
					ERR_LOG("send");
					return -1;
				}
			}
			
		}
	}

	return 0;
}


//查询
int do_select(int cntfd, struct msg recv_msg, sqlite3* user_db, sqlite3* dict_db)
{
	//存储要查询的单词,以及返回的意思
	char word[20] = "";
	char mean[100] = "";
	strcpy(word, recv_msg.text);
	char* errmsg = NULL;
	char sql[N] = "";
	sprintf(sql, "select means from dict where words=\"%s\";", word);
	char** pres = NULL;
	int row, column;
	if(sqlite3_get_table(dict_db, sql, &pres, &row, &column,  &errmsg) != 0)
	{
		//查找失败
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}

	//如果没有找到
	if(0 == row)
	{
		strcpy(mean, "not find");
		recv_msg.type='N';
		sprintf(recv_msg.text, "%s: %s", word, mean);
		if(send(cntfd, &recv_msg, sizeof(recv_msg), 0) <0)
		{
			ERR_LOG("send");
			return -1;
		}
	}
	else //找到
	{
		int i = 0, j=column; 	
		for(i=1; i<row+1; i++)
		{
			j=i*column;
			if(i==row)
			{//如果为最后一个,同时发送结束协议
				recv_msg.type='O';
				char** p=&pres[j];
				history_send(cntfd,recv_msg,word,p,user_db);
				return 0;

			}
			else
			{
				char** p=&pres[j];
				history_send(cntfd,recv_msg,word,p,user_db);
			}
			
		}
	}
	return 0;
}
//保存历史记录
int history_send(int cntfd,struct msg recv_msg,char *word, char **pres,sqlite3* user_db)
{
	sprintf(recv_msg.text, "%s %s", word,*pres);	
	if(send(cntfd, &recv_msg, sizeof(recv_msg), 0) <0)
	{
		ERR_LOG("send");
		return -1;
	}
	char t_time[32] = "";
	time_t t = time(NULL);
	//保存时间信息
	struct tm* info = localtime(&t);
	
	sprintf(t_time, "%d-%02d-%02d %02d:%02d", \
			info->tm_year+1900, info->tm_mon+1, info->tm_mday,\
			info->tm_hour, info->tm_min);
	//保存历史记录
	char* errmsg = NULL;
	char sql[N] = "";
	bzero(sql, sizeof(sql));
	sprintf(sql, "insert into history values(\"%s\", \"%s\", \"%s\", \"%s\")", \
	recv_msg.name, word, *pres, t_time);

	if(sqlite3_exec(user_db, sql, NULL, NULL, &errmsg) != 0)
	{
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}
	return 0;
}


//登录
int do_login(int cntfd, struct msg recv_msg, sqlite3* user_db)
{
	char* errmsg = NULL;
	char sql[N] = "";
	//查找账号和密码
	sprintf(sql, "select * from user where name=\"%s\" and passwd=\"%s\";", recv_msg.name, recv_msg.text);

	char** pres = NULL;
	int row, column;
	if(sqlite3_get_table(user_db, sql, &pres, &row, &column,  &errmsg) != 0)
	{
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}
	if(0 == row)
	{//没有找到
		strcpy(recv_msg.text, "not");
	}
	else
	{
		//判断状态是否是已经登录状态
		if(strcmp(pres[(row+1)*column-1], "0") == 0) 	//未登录状态
		{
			strcpy(recv_msg.text, "success");
			//将状态设置位已经登录状态 stage =1;
			bzero(sql, sizeof(sql));
			sprintf(sql, "update user set stage = 1 where name=\"%s\" ;", recv_msg.name);
			if(sqlite3_exec(user_db, sql, NULL, NULL, &errmsg) != 0)
			{
				printf("%s %d\n", errmsg, __LINE__);
				return -1;
			}
			printf("%s用户登录\n",recv_msg.name);
		}
		else
		{
			strcpy(recv_msg.text, "existence"); 	//已登录状态
		}
	}
	
	if(send(cntfd, &recv_msg, sizeof(recv_msg), 0) <0)
	{
		ERR_LOG("send");
		return -1;
	}

	return 0;
}


//注册
int do_register(int cntfd, struct msg recv_msg, sqlite3* user_db)
{
	char* errmsg = NULL;
	char sql[N] = "";
	sprintf(sql, "insert into user values(\"%s\", \"%s\" , 0);", recv_msg.name, recv_msg.text);

	if(sqlite3_exec(user_db, sql, NULL, NULL, &errmsg) != 0)
	{	
		if(strstr(errmsg, "UNIQUE constraint failed"))
		{//重复注册
			strcpy(recv_msg.text, "existence");
		}
		else
		{//注册失败
			strcpy(recv_msg.text, errmsg);
		}
	}
	else
	{//注册成功		
		strcpy(recv_msg.text, "success");
		printf("%s新用户注册\n",recv_msg.name);
	}

	if(send(cntfd, &recv_msg, sizeof(recv_msg), 0) <0)
	{
		ERR_LOG("send");
		return -1;
	}

	return 0;
}


//网络初始化
int init_socket(int* psfd)
{
	//创建流式套接字
	*psfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*psfd < 0)
	{
		ERR_LOG("socket");
		return -1;
	}
	//允许端口快速重用
	int reues = 1;
	if(setsockopt(*psfd, SOL_SOCKET, SO_REUSEADDR, &reues, sizeof(int))<0)
	{
		ERR_LOG("setsockopt");
		return -1;
	}
	//绑定IP信息
	struct sockaddr_in sin;
	sin.sin_family 		= AF_INET;
	sin.sin_port  		= htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);
	if(bind(*psfd, (void*)&sin, sizeof(sin)) < 0)
	{
		ERR_LOG("bind");
		return -1;
	}
	//设置被动监听状态
	if(listen(*psfd, 10) < 0)
	{
		ERR_LOG("listen");
		return -1;
	}	
	return 0;
}



//数据库初始化
int init_sqlite(sqlite3** pdict_db, sqlite3** puser_db)
{
	char sql[N] = "";
	char* errmsg = NULL;
	//创建打开dict.db数据库
	if(sqlite3_open("./dict.db", pdict_db)!=0)
	{
		printf("%s %d\n", sqlite3_errmsg(*pdict_db), __LINE__);
		return -1;
	}
	//创建dict表格
	sprintf(sql, "create table dict (words char, means char);");
	if((sqlite3_exec(*pdict_db, sql, NULL, NULL, &errmsg)) != 0)
	{
		//如果dict表格存在,则不导入
		if(strcmp("table dict already exists", errmsg) == 0)
		{
		}
		else
		{
			printf("%s %d\n", errmsg, __LINE__);
			return -1;
		}
	}
	//如果dict表格不存在,则创建表格,并将dict.txt导入数据库
	else
	{
		if(do_dict_txt(*pdict_db) !=0)
		{
			return -1;
		}
	}
	printf("电子词典库初始化成功\n");
	//创建打开 user.db数据库 
	if(sqlite3_open("./user.db", puser_db)!=0)
	{
		printf("%s %d\n", sqlite3_errmsg(*puser_db), __LINE__);
		return -1;
	}
	//创建历史记录表
	bzero(sql, sizeof(sql));
	sprintf(sql,"create table if not exists history(name char, words char, means char, time char);");
	if(sqlite3_exec(*puser_db, sql, NULL, NULL, &errmsg) != 0)
	{
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}
	//创建用户注册表格
	bzero(sql, sizeof(sql));
	sprintf(sql,"create table if not exists user(name char primary key, passwd char, stage int);");
	if(sqlite3_exec(*puser_db, sql, NULL, NULL, &errmsg) != 0)
	{
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}
	//清空用户登录状态 0为不在线 1为在线
	bzero(sql, sizeof(sql));
	sprintf(sql,"update user set stage=0;");
	if(sqlite3_exec(*puser_db, sql, NULL, NULL, &errmsg) != 0)
	{
		printf("%s %d\n", errmsg, __LINE__);
		return -1;
	}
	printf("用户信息初始化成功\n");
	return 0;
}

//导入电子词典
int do_dict_txt(sqlite3* dict_db)
{
	char buf[N] = "";
	char word[20] = "";
	char mean[100] = "";
	//打开文件	
	FILE* fp = fopen("./dict.txt", "r");
	if(fp < 0)
	{
		ERR_LOG("fopen");
		return -1;
	}
	//循环读取并插入数据库
	while(fgets(buf, N, fp) != NULL)
	{
		//提取单词和意思
		sscanf(buf, "%s %[^\n]", word,mean);	
		//插入到数据库中
		char sql[N] = "";
		char* errmsg = NULL;
		sprintf(sql, "insert into dict values(\"%s\", \"%s\");", word, mean);
		if(sqlite3_exec(dict_db, sql, NULL, NULL, &errmsg) !=0)
		{
			printf("%s %d\n", errmsg, __LINE__);
			return -1;
		}

		bzero(buf, sizeof(buf));
		bzero(word, sizeof(word));
		bzero(mean, sizeof(mean));
	}
	fclose(fp);
	return 0;
}


