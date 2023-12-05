#include "sta.h"

//修改信息函数
int mod_user_all(int acceptfd,char sql[],MSG *msg,char buf[])
{
    char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__errmsg:%s\n",__LINE__,errmsg);
        strcpy(msg->recvmsg,"error");
       send(acceptfd, msg, sizeof(MSG), 0);
        return -1;
	}
    
	printf("用户数据存储成功\n");
    updata_hidtory(acceptfd,msg,buf);
    strcpy(msg->recvmsg,"OK");
    send(acceptfd, msg, sizeof(MSG), 0);

    return 0;
}

void printf_all(MSG *msg)
{
    printf("%d  %d         %s   %s     %d 	%s  %s  %s  %s  %d  %2f \t\n",
		msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
	
}

