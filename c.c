#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

#define LISTENQ 1024


//server finding
int sendBroadcast()
{
	char msg[128] = "I am client, Where is sever?";
	int brdcFd;
	if((brdcFd = socket(PF_INET, SOCK_DGRAM, 0))== -1){
		printf("socket fail\n");
		return -1;
	}

	int optval = 1;
	setsockopt(brdcFd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));
	struct sockaddr_in theirAddr;
	memset(&theirAddr, 0, sizeof(struct sockaddr_in));
	theirAddr.sin_family = AF_INET;
	theirAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	theirAddr.sin_port = htons(4001);
	int sendBytes;
	if((sendBytes = sendto(brdcFd, msg, strlen(msg), 0, (struct sockaddr *)&theirAddr, sizeof(struct sockaddr))) == -1){
		printf("sendto fail, errno = %d\n", errno);
		return -1;
	}

	printf("msg = %s, msgLen=%d,sendBytes = %d\n", msg, strlen(msg),sendBytes);
	close(brdcFd);

}


//accept() with time out by Chenk
//chenkjiang/article/details/12028009
int accept_timeout(int fd, struct sockaddr *addr, unsigned int wait_seconds){
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);


	if (wait_seconds > 0){
		fd_set accept_fdset;
		struct timeval timeout;
		FD_ZERO(&accept_fdset);
		FD_SET(fd, &accept_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		
		do{
			ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);
		
		if (ret == -1){
			return -1;
		}
		else if(ret == 0){
			errno = ETIMEDOUT;
			return -1;
		}
	}


	if (addr != NULL)
		ret = accept(fd, (struct sockaddr*)addr, &addrlen);
	else
		ret = accept(fd, NULL, NULL );
	/* if (ret == -1)
	ERR_EXIT("accept");
	*/


	return ret;
}



int main()
{
	//-----------------------------------------------------//
	//code from unpv13e
	int listenfd, connfd;
	struct sockaddr_in servaddr;
	char buff[128] = "welcome connected to client.\n";
	char addr[128];

	//very important!
	//must set reuse option!
	//otherwise you have to wait another 30s for os set port free!
	//
	


	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	

	int reuse = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,(char *)&reuse, sizeof(int)) == -1){
		error("cant set reuse option");
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(9000);
	
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);
	

	
	//raw accept()
	//connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

	//accept_timeout
	connfd = -1;
	printf("connfd is here%d\n",connfd);
	while(connfd<=0){
		printf("in while");
		sendBroadcast();
		connfd = accept_timeout(listenfd, (struct sockaddr *)&servaddr, 1);
	}
	printf("after while\n");

	
	inet_ntop(AF_INET,&servaddr.sin_addr,addr,sizeof(addr));	
	printf("Server address is: %s\n",addr);
	//
	//
	//
	//test send message ability
	int i = 0;
	while(i < 10){
		//usleep is 1us as standard. 1,000,000us is 1s
		usleep(100000);
		write(connfd, buff, strlen(buff));
		i++;
	}
//	snprintf(buff,sizeof(buff),"%s\r\n",data);
	close(connfd);
	
	return 0;
}

