#include "sta.h"

int main(int argc, const char *argv[])
{
	//创建流式套接字
	int sfd = socket(AF_INET , SOCK_STREAM ,0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
	}
	//连接服务器
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);
	
	if(connect(sfd,(struct sockaddr *)&sin,sizeof(sin)) < 0)
	{
		ERR_MSG("connect");
		return -1;
	}
	printf("连接成功\n");
	
	do_login(sfd);
	
	
	close(sfd);
	
	return 0;
}