#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXLINE 4096
#define BUFFSIZE 128

char buff[BUFFSIZE];

int recvPacket(){
	int sockListen;
	//char address[128];
	if((sockListen = socket(AF_INET, SOCK_DGRAM,0)) == -1){
		printf("socket fail\n");
		return -1;
	}

	int set = 1;
	setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));
	struct sockaddr_in recvAddr;
	memset(&recvAddr, 0, sizeof(struct sockaddr_in));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(4001);
	recvAddr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockListen, (struct sockaddr *) &recvAddr, sizeof(struct sockaddr))== -1){
		printf("bind fail\n");
		return -1;
	}
	int recvbytes;
	char recvbuf[128];
	int addrLen = sizeof(struct sockaddr_in);
	if((recvbytes = recvfrom(sockListen, recvbuf, 128,0,(struct sockaddr *)&recvAddr, &addrLen))!= -1){
		recvbuf[recvbytes] = '\0';
//		printf("receive a broadcast message:%s\n from %s,port %d", recvbuf,inet_ntop(AF_INET, &recvAddr.sin_addr, recvbuf, sizeof(recvbuf)),
//	 			ntohs(recvAddr.sin_port));
		printf("receive a broadcast message:%s\n", recvbuf);
		

		//in address/buff, there is ip address in 10
		printf("Client detected on:%s\n",inet_ntop(AF_INET,&recvAddr.sin_addr,buff,sizeof(buff)));
	}else{
		printf("recvfrom fail\n");
	}
	close(sockListen);
	return 0;

}

int writeData(){
	//------------------------------------------------------//
	//code from unpv13e
	//used to connect client, to get data from it.
	//sleep(1);
	int sockfd, n;
	char recvline[4096];
	struct sockaddr_in servaddr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM,0))<0){
		printf("socket error");
	}

	//if not enable reuse, then some mistake could happen
	//
	int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1){
		error("cannot set socket reuse opt");
	}



	
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(9000);
		
		//here, the buff is get from last phrase, and it have address
		if(inet_pton(AF_INET, buff,&servaddr.sin_addr) <=0){
			printf("inet_pton error");
		}
		if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
			printf("connect error\n");
			printf("%s\n",buff);
		}
		//
		//
		//at here, first design as standard output, change to memory oper later

		while ( ( n = read(sockfd, recvline, MAXLINE)) > 0){
			recvline[n] = 0;
			
			printf("Message from client%s:",buff);
			if(fputs(recvline, stdout)==EOF){
				printf("fputs error");
			}
		}
		if (n < 0){
			printf("read error\n");
			printf("%d",n);
		}
		return 0;
}

int main(){
	while(1){
		//char * str;
		//str = (char *)malloc(128);
		//char *address[128] = recvPacket();

		recvPacket();

		pid_t pid = fork();
		if(pid > 0){
			//main process
			continue;
		}else if (pid == 0){
			//child process
			printf("writing data: %d\tsuccess\n",writeData());
			return 0;
		}else {
			fprintf(stderr, "fork  error \n");
			return 2;
		}

	}

	exit(0);
	
	return 0;
}
