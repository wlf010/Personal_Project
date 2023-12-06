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
#include <signal.h>
#include <arpa/inet.h>

#define ERR_LOG(send_msg) do{\
	fprintf(stderr, "%d %s ", __LINE__, __func__);\
	perror(send_msg);\
}while(0)

#define IP "192.168.1.106"  //服务器ip信息
#define PORT 8765  //服务器端口信息
#define N 128   //通讯文本存储




typedef struct
{
	int fd ;
	struct sockaddr_in cin;
}CliCnt;

struct msg
{
	int type;
	char name[20];
	char text[N];
};

char name[20] = "";
int sockfd=0; 		
int init_socket(int* psfd);  //网络初始化

int do_quit_cli(int sfd); //退出客户端
int do_history(int sfd); //查历史记录
int do_login(int sfd); //登录
int do_user_menu(int sfd);  //用户交互功能
int do_register(int sfd);  //注册
int do_select(int sfd);  //查单词
int do_quit(int sfd); //退出
//ctrl+c退出
typedef void(*sighandler_t)(int );
void handler(int sig)
{
	printf("\n");
	do_quit(sockfd);
	close(sockfd);
	exit(0);
}


int main(int argc, const char *argv[])
{
	sighandler_t s = signal(2, handler);
	//注册信号处理函数
	if(s == SIG_ERR)
	{
		ERR_LOG("signal");
		return -1;
	}

	/*网络初始化*/
	int sfd = 0;
	if(init_socket(&sfd) < 0)
	{
		return -1;
	}
	//信号处理函数使用,
	//最后加的功能,所有的sfd都可以用sockfd替代
	sockfd = sfd; 		

	while(1)
	{
		system("clear");
		printf("*****************************\n");	
		printf("************1.注册***********\n");	
		printf("************2.登录***********\n");	
		printf("************3.退出***********\n");	
		printf("*****************************\n");

		int choose = 0;
		printf("请输入你的选择:");
		scanf("%d", &choose);
		while(getchar()!=10);

		switch(choose)
		{
			case 1:  //注册功能
				do_register(sfd);
				break;
			case 2:  //登录功能
				if(do_login(sfd) == 0)
				{
					//进入用户界面
					do_user_menu(sfd);
				}
				break;
			case 3:  //退出客户端
				do_quit_cli(sfd);
				return 0;
			default:
				printf("输入错误\n");

		}

		printf("请输入任意字符清屏>>>");
		while(getchar()!=10);
	}
	return 0;
}
//用户交互界面
int do_user_menu(int sfd)
{
	while(1)
	{
		system("clear");
		printf("******************************\n");	
		printf("**********1.查单词************\n");	
		printf("**********2.历史记录**********\n");	
		printf("**********3.退出用户**********\n");	
		printf("******************************\n");
		int choose = 0;
		printf("请输入你的选择:");
		scanf("%d", &choose);
		while(getchar()!=10);

		switch(choose)
		{
			case 1:  //查单词
				do_select(sfd);
				break;
			case 2: //查历史记录
				do_history(sfd);
				break;
			case 3:  //退出用户
				do_quit(sfd);
				return 0;
			default:
				printf("输入错误\n");
		}
		printf("请输入任意字符清屏>>>");
		while(getchar()!=10);
	}

	return 0;
}
//退出客户端
int do_quit_cli(int sfd)
{
	struct msg send_msg ;
	send_msg.type='Q';
	if(send(sfd, &send_msg, sizeof(send_msg), 0)<0)
	{
		ERR_LOG("send");
		return -1;
	}

	close(sfd);
	return 0;
}
//退出用户
int do_quit(int sfd)
{
	struct msg send_msg ;
	send_msg.type='q';
	strcpy(send_msg.name, name);
	//发送退出协议
	if(send(sfd, &send_msg, sizeof(send_msg), 0)<0)
	{
		ERR_LOG("send");
		return -1;
	}
	return 0;
}

//查找历史记录
int do_history(int sfd)
{
	struct msg send_msg ;
	send_msg.type='H';
	strcpy(send_msg.name, name);

	//发送,等待接收历史记录
	if(send(sfd, &send_msg, sizeof(send_msg), 0)<0)
	{
		ERR_LOG("send");
		return -1;
	}

	while(1)
	{
		bzero(send_msg.text, sizeof(send_msg.text));
		int recv_len = recv(sfd, &send_msg, sizeof(send_msg), 0);
		if(recv_len < 0)
		{
			ERR_LOG("recv");
			return -1;
		}
		else if(0 == recv_len)
		{
			printf("服务器关闭\n");
			exit(0);
		}
		printf("%s\n",send_msg.text);

		if(strcmp(send_msg.text, "not find") == 0)
		{//没有找到
			break;
		}
		else if(send_msg.type=='O')
		{//接收到最后一组信息,退出循环
			break;
		}
	}

	return 0;
}


//查询
int do_select(int sfd)
{
	struct msg send_msg ;
	strcpy(send_msg.name, name);

	while(1)
	{
		printf("请输入单词(*退出)>>>");
		scanf("%s", send_msg.text);
		while(getchar()!=10);
		//*号退出
		if(strcmp(send_msg.text, "*") == 0)
		{
			break;
		}

		send_msg.type='S';
		//发送,等待接收意思
		if(send(sfd, &send_msg, sizeof(send_msg), 0)<0)
		{
			ERR_LOG("send");
			return -1;
		}
		while(1)
		{
			bzero(send_msg.text, sizeof(send_msg.text));
			int recv_len = recv(sfd, &send_msg, sizeof(send_msg), 0);
			if(recv_len < 0)
			{
				ERR_LOG("recv");
				return -1;
			}
			else if(0 == recv_len)
			{
				printf("服务器关闭\n");
				exit(0);
			}
			printf("%s\n",send_msg.text);
			if(send_msg.type=='N')
			{//没有找到
				break;
			}
			else if(send_msg.type=='O')
			{//接收到最后一组信息,退出循环
				break;
			}
		}

	}
	return 0;

}
//登录
int do_login(int sfd)
{
	bzero(name, sizeof(name));
	struct msg send_msg;
	send_msg.type='L';
	printf("请输入登录账户名>>>");
	scanf("%s", send_msg.name);
	strcpy(name, send_msg.name );
	while(getchar()!=10);

	printf("请输入登录密码>>>");
	scanf("%s", send_msg.text);
	while(getchar()!=10);

	//发送,等待接收应答
	if(send(sfd, &send_msg, sizeof(send_msg), 0)<0)
	{
		ERR_LOG("send");
		return -1;
	}

	memset(&send_msg, 0, sizeof(send_msg));
	int recv_len = recv(sfd, &send_msg, sizeof(send_msg), 0);
	if(recv_len < 0)
	{
		ERR_LOG("recv");
		return -1;
	}
	else if(0 == recv_len)
	{
		printf("服务器关闭\n");
		exit(0);
	}

	if(strcmp(send_msg.text, "success") == 0)
	{
		printf("登录成功!\n");
	}
	else if(strcmp(send_msg.text, "existence") == 0) 
	{
		printf("登录失败,重复登录\n");
		return -1;
	}
	else
	{
		printf("登录失败,账号或密码错误\n");
		return -1;
	}
	return 0;
}


//注册 
int do_register(int sfd)
{
	struct msg send_msg;
	send_msg.type='R';
	printf("请输入账户名>>>");
	scanf("%s", send_msg.name);
	while(getchar()!=10);

	printf("请输入密码>>>");
	scanf("%s", send_msg.text);
	while(getchar()!=10);
	int i=0;
	char text[N]="";
	strcpy(text,send_msg.name);
	for(i=0;i<20;i++)
	{
		if(text[i]=='\"')
		{
			printf("用户名非法\n");
			return 0;
		}

	}
	strcpy(text,send_msg.text);
	for(i=0;i<20;i++)
	{
		if(text[i]=='\"')
		{
			printf("密码非法\n");
			return 0;
		}

	}
	//发送,等待接收应答
	if(send(sfd, &send_msg, sizeof(send_msg), 0)<0)
	{
		ERR_LOG("send");
		return -1;
	}

	memset(&send_msg, 0, sizeof(send_msg));
	int recv_len = recv(sfd, &send_msg, sizeof(send_msg), 0);
	if(recv_len < 0)
	{
		ERR_LOG("recv");
		return -1;
	}
	else if(0 == recv_len)
	{
		printf("服务器关闭\n");
		return 0;
	}

	if(strcmp(send_msg.text, "success") == 0)
	{
		printf("注册成功!\n");
	}
	else if(strcmp(send_msg.text, "existence") == 0)
	{
		printf("用户已存在\n");
	}
	else
	{
		printf("%s\n",send_msg.text);
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
	int reues = 1 ;
	if(setsockopt(*psfd, SOL_SOCKET, SO_REUSEADDR, &reues, sizeof(int))<0)
	{
		ERR_LOG("setsockopt");
		return -1;
	}


	//填充服务器信息
	struct sockaddr_in sin;
	sin.sin_family 		= AF_INET;
	sin.sin_port  		= htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);

	if(connect(*psfd, (void*)&sin, sizeof(sin)) < 0)
	{
		ERR_LOG("bind");
		return -1;
	}
	
	return 0;
}


