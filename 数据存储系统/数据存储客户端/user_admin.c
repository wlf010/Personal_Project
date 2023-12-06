#include "sta.h"


//登录请求
int admin_or_user_login(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//输入用户名和密码
	memset(msg->username, 0, NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();

	memset(msg->passwd, 0, DATALEN);
	printf("请输入密码（6位）");
	scanf("%s",msg->passwd);
	getchar();

	printf("\033c");

	//发送登陆请求
	send(sockfd, msg, sizeof(MSG), 0);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	
	printf("******当前操作人员:%s **********\n",msg->username);

	//判断是否登陆成功
	if(strncmp(msg->recvmsg, "OK", 2) == 0)
	{
		if(msg->usertype == ADMIN)
		{
			printf("亲爱的管理员，欢迎您登陆员工管理系统！\n");
			admin_menu(sockfd,msg);
		}
		else if(msg->usertype == USER)
		{
			printf("亲爱的用户，欢迎您登陆员工管理系统！\n");
			user_menu(sockfd,msg);
		}
	}
	else
	{
		printf("登陆失败！%s\n", msg->recvmsg);
		return -1;
	}
	return 0;
}
/**
查询用户
**/
void do_admin_user_query(int sockfd,MSG *msg)
{
	if(msg->msgtype == ADMIN_QUERY)
	{
		printf("请输入你需要查询的用户名>>>\n");
		char name[NAMELEN]={};
		scanf("%s",name);
		getchar() !='\n';
		strcpy(msg->recvmsg,name);
	
	}
	


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


//修改用户资料
void do_user_admin_modification(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	int a = 0;

	if(msg->msgtype == ADMIN_MODIFY)
	{
		printf("请输入你需要修改的用户>>>");
		char name[NAMELEN]={};
		scanf("%s",name);
		getchar() !='\n';
		strcpy(msg->recvmsg,name);
	}


		while(1)
		{
			
			if(msg->msgtype == USER_MODIFY)
			{
				do_user_modification_a();
				scanf("%d",&n);
				getchar();
				printf("\033c");
				if(n == 1)
				{
					n = 4;
				}
				else if(n == 2)
				{
					n = 5;
				}
				else if(n == 3)
				{
					n = 6;
				}
				else if(n == 4)
				{
					n = 7;
				}
				
			}
			else if(msg->msgtype == ADMIN_MODIFY)
			{
				do_admin_modification_a();
				scanf("%d",&n);
				getchar();
				printf("\033c");
			}
				switch (n)
				{
					case  0:
					a= 1;
						break;
				case  1:
				msg->flags = 1;
					do_user_mod_no(sockfd,msg);
					break;
				case  2:
				msg->flags = 2;
					do_user_mod_usertype(sockfd,msg);
					break;
				case  3:
				msg->flags = 3;
					do_user_mod_name(sockfd,msg);
					break;
				case  4:
				msg->flags = 4;
					do_user_mod_passwd(sockfd,msg);
					break;
				case  5:
				msg->flags = 5;
					do_user_mod_age(sockfd,msg);
					break;
				case  6:
				msg->flags = 6;
					do_user_mod_phone(sockfd,msg);
					break;
				case  7:
				msg->flags = 7;
					do_user_mod_addr(sockfd,msg);
					break;
				case  8:
				msg->flags = 8;
					do_user_mod_work(sockfd,msg);
					break;
				case  9:
				msg->flags = 9;
					do_user_mod_date(sockfd,msg);
				break;
				case  10:
				msg->flags = 10;
					do_user_mod_level(sockfd,msg);
				break;
				case 11:
				msg->flags = 11;
					do_user_mod_salary(sockfd,msg);
				break;

				default:
					printf("输入错误请重新输入\n");
					break;
				}						
				if(a == 1)
					break;
		}

}


//想服务器发送修改的数据，并判断是否成功(int 类型)
int updata_user_mod_int(int socked,MSG *msg)
{
	
	int b;
	scanf("%d",&b);
	getchar();
	if( b == 1)
	{
	
		send(socked, msg, sizeof(MSG), 0);
		recv(socked, msg, sizeof(MSG), 0);
		if(strncmp(msg->recvmsg, "OK", 2) == 0)
		{
			printf("修改成功\n");
			return 0;
		}
		else
		{
			printf("修改失败\n");
			return -1;

		}
	}
	else 
	{
		printf("退出\n");
		return 0;
	}
}
int do_user_mod_no(int sockfd,MSG *msg)
{
	int a;
	printf("请输入 new no \n");
	scanf("%d",&a);
	getchar();

	printf("修改后的 no 为 %d  确定吗?(1.确定 2.退出)\n",a);
	msg->info.no = a;
	updata_user_mod_int(sockfd,msg);
	
}
int do_user_mod_usertype(int sockfd,MSG *msg)
{
	int a;
	printf("请输入 new usertype (1.普通权限 0.管理员权限)\n");
	scanf("%d",&a);
	getchar();
	
	printf("修改后的 usertype 为 %d  确定吗?(1.确定 2.退出)\n",a);
	msg->info.usertype = a;
	updata_user_mod_int(sockfd,msg);
	
}

int do_user_mod_age(int sockfd,MSG *msg)
{
	int a;
	printf("请输入 new age \n");
	scanf("%d",&a);
	getchar();

	printf("修改后的 age 为 %d  确定吗?(1.确定 2.退出)\n",a);
	msg->info.age= a;
	updata_user_mod_int(sockfd,msg);
}

int do_user_mod_level(int sockfd,MSG *msg)
{
	int a;
	printf("请输入 new level (范围 1～ 5) \n");
	scanf("%d",&a);
	getchar();

	printf("修改后的 level 为 %d  确定吗?(1.确定 2.退出)\n",a);
	msg->info.level= a;
	updata_user_mod_int(sockfd,msg);
}
int do_user_mod_salary(int sockfd,MSG *msg)
{
	double a;
	printf("请输入 new salary ) \n");
	scanf("%2lf",&a);
	getchar();

	printf("修改后的 salary 为 %2lf  确定吗?(1.确定 2.退出)\n",a);
	msg->info.salary= a;
	updata_user_mod_int(sockfd,msg);
}

//想服务器发送修改的数据，并判断是否成功(char 类型)
int updata_user_mod_char(int sockfd,MSG *msg)
{
	int b;
	scanf("%d",&b);
	getchar();

	if( b == 1)
	{
		
		send(sockfd, msg, sizeof(MSG), 0);
		recv(sockfd, msg, sizeof(MSG), 0);
		if(strncmp(msg->recvmsg, "OK", 2) == 0)
		{
			printf("修改成功\n");
			return 0;
		}
		else
		{
			printf("修改失败\n");
			return -1;

		}
	}
	else 
	{
		printf("退出\n");
		return 0;
	}
}

int do_user_mod_name(int sockfd,MSG *msg)
{
	char new[NAMELEN]={};
	printf("请输入 new name \n");
	scanf("%s",new);
	getchar();

	printf("修改后的 name 为 %s  确定吗?(1.确定 2.退出)\n",new);
	strcpy(msg->info.name,new);
	updata_user_mod_char(sockfd,msg);
}

int do_user_mod_passwd(int sockfd,MSG *msg)
{
	char new[NAMELEN]={};
	printf("请输入 new passwd \n");
	scanf("%s",new);
	getchar();

	printf("修改后的 passwd 为 %s  确定吗?(1.确定 2.退出)\n",new);
	strcpy(msg->info.passwd,new);
	updata_user_mod_char(sockfd,msg);
}

int do_user_mod_phone(int sockfd,MSG *msg)
{
	char new[NAMELEN]={};
	printf("请输入 new phone \n");
	scanf("%s",new);
	getchar();

	printf("修改后的 phone 为 %s  确定吗?(1.确定 2.退出)\n",new);
	strcpy(msg->info.phone,new);
	updata_user_mod_char(sockfd,msg);
}

int do_user_mod_addr(int sockfd,MSG *msg)
{
	char new[NAMELEN]={};
	printf("请输入 new addr \n");
	scanf("%s",new);
	getchar();

	printf("修改后的 addr 为 %s  确定吗?(1.确定 2.退出)\n",new);
	strcpy(msg->info.addr,new);
	updata_user_mod_char(sockfd,msg);
}

int do_user_mod_work(int sockfd,MSG *msg)
{
	char new[NAMELEN]={};
	printf("请输入 new work \n");
	scanf("%s",new);
	getchar();

	printf("修改后的 work 为 %s  确定吗?(1.确定 2.退出)\n",new);
	strcpy(msg->info.work,new);
	updata_user_mod_char(sockfd,msg);
}

int do_user_mod_date(int sockfd,MSG *msg)
{
	char new[NAMELEN]={};
	printf("请输入 new date \n");
	scanf("%s",new);
	getchar();

	printf("修改后的 date 为 %s  确定吗?(1.确定 2.退出)\n",new);
	strcpy(msg->info.date,new);
	updata_user_mod_char(sockfd,msg);
}

void printf_all(MSG *msg)
{
printf("%d  %d         %s   %s     %d 	%s  %s  %s  %s  %d  %2f \t\n",
		msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
	


}