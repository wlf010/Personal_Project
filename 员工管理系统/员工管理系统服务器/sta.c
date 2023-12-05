#include "sta.h"

int process_client_request(int newfd,MSG *msg)
{
		char sql[1024]={};
    printf("------------%s-----------%d.\n",__func__,__LINE__);
	printf("msg->recvmsg :%s\n",msg->recvmsg);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(newfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(newfd,msg);
			break;
		case USER_QUERY:			
			sprintf(sql,"select * from usrinfo where name =\"%s\";",msg->username);
			process_user_query_request(newfd,msg,sql);
			break;
		case ADMIN_MODIFY:
			process_user_modify_request(newfd,msg);
			break;
		case ADMIN_ADDUSER:
			process_admin_adduser_request(newfd,msg);
			break;
		case ADMIN_DELUSER:
			process_admin_deluser_request(newfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(newfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(newfd,msg);
			break;
		case QUIT:
			//process_client_quit_request(newfd,msg);
			break;
		default:
			break;
	}

}
/**************************************
 *函数名：process_user_or_admin_login_request
 *参   数：
 *功   能：用户和管理员登录
 ****************************************/
int process_user_or_admin_login_request(int newfd,MSG *msg)
{
    printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[1024] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	
	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name ,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	
    sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	char buf[DATALEN]={"登录"};
    // nrow 满足条件的记录数
    if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"账号或密码错误\n");
			send(newfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(newfd,msg,sizeof(MSG),0);
			bzero(msg->recvmsg,0);
			//同时将成功操作信息存入历史集记录表格
			updata_hidtory(newfd,msg,buf);
		}
	}
	return 0;	
}
/**************************************
 *函数名：process_user_query_request
 *参   数：
 *功   能：用户查询自身信息
 ****************************************/
int process_user_query_request(int acceptfd,MSG *msg,char sql[])
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char** result = NULL;
	int row, column;
	char* errmsg = NULL;
	char buf[DATALEN]={"查询自身用户信息"};
	
	if(sqlite3_get_table(db, sql, &result , &row, &column, &errmsg)  != 0)
	{   
		printf("__%d__ errmsg:%s\n", __LINE__, errmsg);
	}
		
	if(row != 0)
	{
			int line = 0, list=0;
			int j = 0;

			for(line = 0 ;line < (row+1)*column;line++ ,j++)
			{
				if(j > 10)
				{
					if((j+1)%11==0)
					{						
						sprintf(msg->recvmsg,"%s %s %s %s %s %s %s %s %s %s %s \t",result[j-10],result[j-9],result[j-8],result[j-7],result[j-6],result[j-5],result[j-4],result[j-3],result[j-2],result[j-1],result[j]);
						memset(&msg->info,0,sizeof(staff_info_t));
						msg->info.no =atoi(result[j-10]);
						msg->info.usertype =atoi(result[j-9]);
						strcpy(msg->info.name , result[j -8]);
						strcpy(msg->info.passwd,result[j-7]);
						msg->info.age = atoi(result[j-6]);
						strcpy(msg->info.phone,result[j-5]);
						strcpy(msg->info.addr,result[j-4]);
						strcpy(msg->info.work,result[j-3]);
						strcpy(msg->info.date,result[j-2]);
						msg->info.level = atoi(result[j-1]);
						msg->info.salary =atoi(result[j]);					
						msg->flags = 14;
						
						send(acceptfd, msg, sizeof(MSG), 0);
						printf_all(msg);
						
					}
				}
			
			}
		memset(&msg->info,0,sizeof(staff_info_t));
		printf("--%s--\n",msg->recvmsg);
		strcpy(msg->recvmsg,"ok");
		send(acceptfd, msg, sizeof(MSG), 0);
			
	}
	updata_hidtory(acceptfd,msg,buf);
	sqlite3_free_table(result);

}
/**************************************
 *函数名：process_user_modify_request
 *参   数：
 *功   能：用户修改自身信息
 ****************************************/
int process_user_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char data[NAMELEN]={};
	char sql[256]={};
	char buf[128]={};
	printf("-------------%s______\n",msg->recvmsg);
	strcpy(msg->info.name,msg->recvmsg);
	switch(msg->flags)
	{
		case 1 :
			strcpy(data,"staffno");
			sprintf(buf,"修改 %s   no",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%d\" where name= \"%s\";",data,msg->info.no,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
		break;
		case 2:
			strcpy(data,"usertye");
			sprintf(buf,"修改 %s usertye",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%d\" where name= \"%s\";",data,msg->info.usertype,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
		
		break;
		case 3:
			strcpy(data,"name");
			sprintf(buf,"修改 %s name",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%s\" where name= \"%s\";",data,msg->info.name,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
			break;
		case 4:
			strcpy(data,"passwd");
			sprintf(buf,"修改 %s passwd",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%s\" where name= \"%s\";",data,msg->info.passwd,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
			break;
		case 5:
			strcpy(data,"age");
			sprintf(buf," 修改 %s age",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%d\" where name= \"%s\";",data,msg->info.age,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
		break;
		case 6:
					strcpy(data,"phone");
					sprintf(buf,"修改 %s  phone",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%s\" where name= \"%s\";",data,msg->info.phone,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
		break;
		case 7:
				strcpy(data,"addr");
				sprintf(buf,"修改 %s addr",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%s\" where name= \"%s\";",data,msg->info.addr,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
		break;
		case 8:
			strcpy(data,"work");
			sprintf(buf,"修改 %s work",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%s\" where name= \"%s\";",data,msg->info.work,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
		break;
		case 9:
			strcpy(data,"date");
			sprintf(buf,"修改 %s date",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%s\" where name= \"%s\";",data,msg->info.date,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
		break;
		case 10:
			strcpy(data,"level");
			sprintf(buf,"修改 %s level",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%d\" where name= \"%s\";",data,msg->info.level,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
		break;
		case 11:
			strcpy(data,"salary");
			sprintf(buf,"修改  %s alary",msg->info.name);
			sprintf(sql,"update usrinfo SET \"%s\"=\"%2lf\" where name= \"%s\";",data,msg->info.salary,msg->info.name);
			mod_user_all(acceptfd,sql,msg,buf);
			
			break;

	}


}

//指定用户查寻
int process_admin_query_request(int acceptfd,MSG *msg)
{
	//查询个人用户
	if(msg->flags == 12)
	{
		//strcpy(msg->info.name , msg->recvmsg);
		char sql[512]={};
		char buf[128]={};
		sprintf(sql,"select * from usrinfo where name =\"%s\";",msg->recvmsg);
		sprintf(buf,"查询  %s 的信息",msg->recvmsg);
		updata_hidtory(acceptfd,msg,buf);
		process_user_query_request(acceptfd,msg,sql);
	}
	//查询所有用户
	else if(msg->flags == 13)
	{
		//打印表中所有数据
		char sql[128]={};
		sprintf(sql,"select * from usrinfo;");
		char buf[DATALEN]={"查询所有的信息"};
		updata_hidtory(acceptfd,msg,buf);
		process_user_query_request(acceptfd,msg,sql);

	}
}
//添加新员工
int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[1024]={};
	printf_all(msg);
	sprintf(sql,"insert into usrinfo values(\"%d\",\"%d\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%2f\")",msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
	char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__errmsg:%s\n",__LINE__,errmsg);
		return -1;
	}
	else
	{
		strcpy(msg->recvmsg,"ok");
		send(acceptfd,msg,sizeof(MSG),0);
		char buf[DATALEN]={"添加新员工"};
		updata_hidtory(acceptfd,msg,buf);

	}
	
}
//删除员工
int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[1024]={};
	char *errmsg = NULL;
	sprintf(sql,"DELETE FROM usrinfo where name=\"%s\";",msg->info.name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__errmsg:%s\n",__LINE__,errmsg);
		return -1;
	}
	else
	{
		strcpy(msg->recvmsg,"ok");
		send(acceptfd,msg,sizeof(MSG),0);
		char buf[DATALEN]={"删除员工"};
		updata_hidtory(acceptfd,msg,buf);

	}


}

//存入历史记录表中
int updata_hidtory(int acceptfd,MSG * msg,char work[])
{
	printf("当前操作人员为 ：%s \n",msg->username);
	time_t t;
	t = time(NULL);
	struct tm* info = localtime(&t);
	if(NULL == info)
	{   
		perror("localtime");
		return -1; 
	}   
	sprintf(msg->time,"%d-%02d-%02d %02d-%02d-%02d\n", info->tm_year+1900, info->tm_mon+1, info->tm_mday, \
			info->tm_hour, info->tm_min, info->tm_sec);
	//将记录插入historybiao
	char sql[1024]="";
	sprintf(sql,"insert into historyinfo values(\"%s\",\"%s\",\"%s\");",msg->time,msg->username,work);
	char *errmsg = NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__errmsg:%s\n",__LINE__,errmsg);
		return -1;
	}

}

//查询历史记录
int process_admin_history_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

			char sql[128]={};
			sprintf(sql,"select * from historyinfo;");
			char** result = NULL;
			int row, column;
			char* errmsg = NULL;
				if(sqlite3_get_table(db, sql, &result , &row, &column, &errmsg)  != 0)
	{   
		printf("__%d__ errmsg:%s\n", __LINE__, errmsg);
	}
		
	if(row != 0)
	{
			int line = 0, list=0;
			int j = 0;

			for(line = 0 ;line < (row+1)*column;line++ ,j++)
			{
				if(j > 3)
				{
					if((j+1)%3==0)
					{						
						sprintf(msg->time,"%s %s %s  \t",result[j-2],result[j-1],result[j]);
						memset(&msg->info,0,sizeof(staff_info_t));		
						msg->flags = 14;
						
						send(acceptfd, msg, sizeof(MSG), 0);
						printf_all(msg);
						
					}
				}
			
			}
		memset(&msg->info,0,sizeof(staff_info_t));
		printf("--%s--\n",msg->recvmsg);
		strcpy(msg->recvmsg,"ok");
		send(acceptfd, msg, sizeof(MSG), 0);
			
	}
	sqlite3_free_table(result);

}
