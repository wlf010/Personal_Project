#include "sta.h"

void* recv_cli_msg(void* arg)
{
    //分离线程
    pthread_detach(pthread_self());
	MSG * msg =NULL;
	msg = (MSG *)malloc(sizeof(MSG));
    allmsg cliInfo 		= *(allmsg*)arg;
	int newfd 				= cliInfo.newfd;
	struct sockaddr_in cin 	= cliInfo.cin;

    printf("等待客户端传送数据中\n");

	while (1)
	{
		
		ssize_t res = recv(newfd, msg, sizeof(MSG), 0);
		if(res < 0)
		{
			perror("recv");
		}
		else if(0 == res)
		{	
			printf("客户端关闭\n");
			break;
		}
		
		

		process_client_request(newfd,msg);
		
	}
}


int main()
{

	//创建流式套接字
	int sfd = socket(AF_INET , SOCK_STREAM ,0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
	}
	
	//允许端口快速复用
	int reuse = 1;
	if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse)) < 0)
	{
		perror("setsockopt");
		return -1;
	}
	
	//绑定服务器的ip和端口bind
	struct sockaddr_in sin;
	sin.sin_family  = AF_INET;
	sin.sin_port  = htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);
	
	if(bind(sfd,(struct sockaddr *)&sin,sizeof(sin)) < 0)
	{
		perror("bind");
		return -1;
	}
	
	//设置为被动监听状态
	if(listen(sfd,10) < 0)
	{
		perror("listen");
		return -1;
	}

	//连接客户端的地址信息
	struct sockaddr_in cin;
	socklen_t addrlen = sizeof(cin);

	int newfd;
	pthread_t tid = -1;
	allmsg cliInfo;
	
	while(1)
	{
		//主线程负责链接
		 newfd= accept(sfd, (struct sockaddr*)&cin, &addrlen);
		if(newfd < 0)
		{
			perror("accept");
			return -1;
		}


        //链接成功创建分支线程进行交互
        cliInfo.newfd = newfd;
		cliInfo.cin = cin;
		if(pthread_create(&tid, NULL, recv_cli_msg, (void*)&cliInfo) != 0)
		{
			perror("pthread_create");
			return -1;
		}

				//打开数据库
		if(sqlite3_open("./staff_manage_system.db",&db) != SQLITE_OK){
			printf("%s.\n",sqlite3_errmsg(db));
		}else{
			printf("the database open success.\n");
		}

	//执行数据库操作

	//打开历史记录数据表
		
		char sql[128] = "create table if not exists historyinfo(time char, name char , words char);";
		char *errmsg = NULL;
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
			printf("%s.\n",errmsg);
		}else{
			printf("create historyinfo table success.\n");
		}

		//打开用户数据表
		char sql1[256] = "create table if not exists usrinfo(staffno int primary key ,usertype int ,name char,passwd char ,age int ,phone char,addr char,work char ,date ,char ,level int ,salary double);";
		char *errmsg1 = NULL;
		if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg1)!= SQLITE_OK){
			printf("%s.\n",errmsg1);
		}else{
			printf("create usrinfo table success.\n");
		}


	}
	
    close(sfd);

    return 0;
}

