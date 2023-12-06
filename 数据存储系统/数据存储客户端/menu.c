
#include "sta.h"
//登录菜单
int  do_login(int sockfd)
{
    int n;
    MSG * msg =NULL;
	msg = (MSG *)malloc(sizeof(MSG));
    while (1)
    {
            printf("*******************************************\n");
            printf("                1.管理员模式\n");
            printf("                2.普通用户模式\n");
            printf("                3.退出\n");
            printf("*******************************************\n"); 
			scanf("%d",&n);
			getchar();
			printf("\033c");

        switch(n)
		{
			case 1:
				msg->msgtype  = ADMIN_LOGIN;
				msg->usertype = ADMIN;
				break;
			case 2:
				msg->msgtype =  USER_LOGIN;
				msg->usertype = USER;
				break;
			case 3:
				msg->msgtype = QUIT;
				if(send(sockfd, msg, sizeof(MSG), 0)<0)
				{
					perror("do_login send");
					return -1;
				}
				free(msg);
				close(sockfd);
				exit(0);
			default:
				printf("您的输入有误，请重新输入\n"); 
		}

		admin_or_user_login(sockfd,msg);

    }
    
}

/**************************************
 *函数名：admin_menu
 *参   数：消息结构体
 *功   能：管理员菜单
 ****************************************/
void admin_menu(int sockfd,MSG *msg)
{
		printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n=0;
	while (1)
	{
		int n;
		admin_menu_a();
		scanf("%d",&n);
		getchar();
		printf("\033c");

		switch (n)
		{
		case 1:
			msg->flags = 12;
			msg->msgtype =ADMIN_QUERY;
			do_admin_user_query(sockfd,msg);
			break;
		case 2:	
			msg->flags =13;
			do_admin_query_all(sockfd,msg);		
			break;
		case 3:
			do_admin_history(sockfd,msg);
			break;
		case 4:
		do_admin_adduser(sockfd,msg);
			break;
		case 5:
		do_admin_deluser(sockfd,msg);
			break;
		case 6:
			msg->msgtype = ADMIN_MODIFY;
		do_user_admin_modification(sockfd,msg);
			break;
		case 7:
			n = 1;
			break;
		default:
			printf("输入错误请重新输入\n");
			break;
		}
		
		if(n == 1)
		{
			break;
		}
		

	}
	
}


void user_menu_a()
{
    printf("****************************************\n");
    printf("            1.查询信息\n");
    printf("            2.修改信息\n");
    printf("            3.返回上一级\n");
    printf("****************************************\n");
}

void do_user_modification_a()
{
	printf("********************************\n");
	printf(" 1. 修改 passwd\n");
	printf(" 2. 修改 age\n");
	printf(" 3. 修改 phone\n");
	printf(" 4. 修改 addr\n");
	printf(" 0.退出修改\n");
	printf("********************************\n");
}

void admin_menu_a()
{
	printf("****************************\n");
	printf("		1.查询的指定员工\n");
	printf("		2.查询所有员工信息\n");
	printf("		3.查询历史记录\n");
	printf("		4.添加新员工信息\n");
	printf("		5.删除离职员工\n");
	printf("		6.修改员工信息\n");
	printf("		7.退出\n");
	printf("****************************\n");

}


void do_admin_modification_a()
{
	printf("********************************\n");
	printf(" 1.  修改 no\n");
	printf(" 2.  修改  usertype\n");
	printf(" 3.  修改 name\n");
	printf(" 4.  修改 passwd\n");
	printf(" 5.  修改 age\n");
	printf(" 6.  修改 phone\n");
	printf(" 7.  修改 addr\n");
	printf(" 8.  修改 work\n");
	printf(" 9.  修改 date\n");
	printf(" 10. 修改 level\n");
	printf(" 11. 修改 salary\n");
	printf(" 0.  退出修改\n");
	printf("********************************\n");
}
