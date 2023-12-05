#include "sta.h"
void clear_all()
{
	int a;
	printf("输入任意键结束\n");
	scanf("%d",&a);
	while(getchar() !='\n');
	printf("\033c");
}

//查询所有用户
void do_admin_query_all(int sockfd,MSG *msg)
{
		msg->msgtype = ADMIN_QUERY;
		send(sockfd, msg, sizeof(MSG), 0);
		printf("no    usertype  name   passwd     age	 phone 	addr      work  date  	level   salary  \t \n");
		while(1)
		{
				recv(sockfd, msg, sizeof(MSG), 0);
				
				if(strncmp(msg->recvmsg,"ok",2) == 0)
					{
						
							break;
						
					}
					else if(msg->flags == 14)
					{
							printf("%d  %d         %s   %s     %d 	%s  %s  %s  %s  %d  %2f \t\n",
							msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
					}

		}
	clear_all();
}
/**************************************
 *函数名：admin_adduser
 *参   数：消息结构体
 *功   能：管理员创建用户
 ****************************************/
void do_admin_adduser(int sockfd,MSG *msg)//管理员添加用户
{		
	printf("------------%s-----------%d.\n",__func__,__LINE__);

	msg->msgtype = ADMIN_ADDUSER;

	printf("请输入员工信息 \n");
	int a;
	printf("请输入 new no \n");
	scanf("%d",&a);
	while(getchar() !='\n');
	msg->info.no = a;
		
	int b;
	printf("请输入 new usertype (1 或 0) \n");
	scanf("%d",&b);
	while(getchar() !='\n');
	msg->info.usertype = b;

	char name[NAMELEN]={};
	printf("请输入 new name \n");
	scanf("%s",name);
	getchar();
	strcpy(msg->info.name,name);

	char passwd[NAMELEN]={};
	printf("请输入 new passwd (6 位密码) \n");
	scanf("%s",passwd);
	getchar();
	strcpy(msg->info.passwd,passwd);

	int age;
	printf("请输入 new age \n");
	scanf("%d",&age);
	while(getchar() !='\n');
	msg->info.age = age;

	char phone[NAMELEN]={};
	printf("请输入 new phone \n");
	scanf("%s",phone);
	getchar();
	strcpy(msg->info.phone,phone);

	char addr[NAMELEN]={};
	printf("请输入 new addr \n");
	scanf("%s",addr);
	getchar();
	strcpy(msg->info.addr,addr);

	char work[NAMELEN]={};
	printf("请输入 new work \n");
	scanf("%s",work);
	getchar();
	strcpy(msg->info.work,work);

	char date[NAMELEN]={};
	printf("请输入 new date \n");
	scanf("%s",date);
	getchar();
	strcpy(msg->info.date,date);

		int level;
	printf("请输入 new level \n");
	scanf("%d",&level);
	while(getchar() !='\n');
	msg->info.level = level;

	double salary;
	printf("请输入 new salary \n");
	scanf("%2lf",&salary);
	while(getchar() !='\n');
	msg->info.salary = salary;

	send(sockfd,msg,sizeof(MSG),0);
	printf_all(msg);
	recv(sockfd,msg,sizeof(MSG),0);
	if(strncmp(msg->recvmsg ,"ok",2) == 0)
	{
		printf("添加新员工成功\n");

	}
	else
	{
		printf("添加失败\n");
	}
	

}
//删除员工
void do_admin_deluser(int sockfd,MSG *msg)
{
	msg->msgtype = ADMIN_DELUSER;
	printf("请输入你删除的用户名\n");
	char name[NAMELEN]={};
	scanf("%s",name);
	while(getchar() !='\n');

	printf("确定将  %s 用户删除吗？  确定吗?(1.确定 2.退出)\n",name);
	int a;
	scanf("%d",&a);
	getchar();

		if(a == 1)
		{
			strcpy(msg->info.name,name);
			
			send(sockfd,msg,sizeof(MSG),0);
			recv(sockfd,msg,sizeof(MSG),0);
			
			if(strncmp(msg->recvmsg ,"ok",2) == 0)
			{
				printf("删除员工成功\n");

			}
			else
			{
				printf("删除失败\n");
			}

		}
		else
		{
			
		}


}

/**************************************
 *函数名：admin_modification
 *参   数：消息结构体
 *功   能：管理员修改
 ****************************************/
void do_admin_modification(int sockfd,MSG *msg)//管理员修改
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	msg->msgtype = ADMIN_MODIFY;
	do_user_admin_modification(sockfd,msg);
	

}
//查询历史记录
void do_admin_history(int sockfd,MSG *msg)
{	
	msg->msgtype = ADMIN_HISTORY;

	send(sockfd, msg, sizeof(MSG), 0);
	printf(" 	TIME 		USER		WORK\n");
	while(1)
		{
				recv(sockfd,msg , sizeof(MSG), 0);
				
				if(strncmp(msg->recvmsg,"ok",2) == 0)
					{
						
							break;
						
					}
					else if(msg->flags == 14)
					{
							printf("%s\n",msg->time);
							

					}

		}
	clear_all();

}