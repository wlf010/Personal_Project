#include "sta.h"
/**************************************
 *函数名：user_menu
 *参   数：消息结构体
 *功   能：员工菜单
 ****************************************/
int user_menu(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

	int  a = 0;
	while (1)
	{
		int n;
		user_menu_a();
		scanf("%d",&n);
		getchar();
		printf("\033c");

		switch (n)
		{
			case 1:
			msg->msgtype = USER_QUERY;
			//将recvmsg作为信息载体
			strcpy(msg->recvmsg,msg->username);
			do_admin_user_query(sockfd,msg);
			break;
		case 2:
			msg->msgtype = USER_MODIFY;
			strcpy(msg->recvmsg,msg->username);
			do_user_admin_modification(sockfd,msg);
			break;
		case 3:
			a = 1;
			break;
		default:
			printf("输入错误请重新输入\n");
			break;
		}
		if(a == 1)
		{
			break;
		}

	}
	
	
	return 0;
	
}


