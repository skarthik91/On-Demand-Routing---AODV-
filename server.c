#include "hw_addrs.h"
 
#define SERVERPATH "/tmp/karthikeyan"
#define ODR_PATH "/tmp/ashanatayal"

int msg_rcv(int sockfd,char messagercvd[],char ClientCanonicalIP[],int *port)
{

	int rcvdbytes,k=0;
	struct sockaddr_un odraddr;
	char buffer[MAXLINE];
	socklen_t len;
	char msgrcvd[8][MAXLINE];
	rcvdbytes = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr*) &odraddr, &len);
	
	if(rcvdbytes < 0)
    {
        printf("recvfrom error \n");
		return 0;
    }else{
		printf("Message Received from client is %s \n",buffer);
		char* token = strtok(buffer, " ");
		while(token != NULL) {
        strcpy(msgrcvd[k],token);
        k++;
        token = strtok(NULL, " ");
		}
		strcpy(messagercvd,msgrcvd[7]);
		strcpy(ClientCanonicalIP,msgrcvd[1]);
		printf("port %s \n",msgrcvd[2]);
		*port = atoi(msgrcvd[2]);
		
		return 1;
	}
    return 0;
}

 int msg_send(int sockfd, char ClientCanonicalIP[], int destport, char msgtosend[], int flag)
{
		char packet[MAXLINE];
		struct sockaddr_un odraddr,addr;
		int sendbytes;
		
		snprintf(packet, sizeof(packet), "%d / %s / %d / %s / %d", sockfd,ClientCanonicalIP,destport,msgtosend,flag);
		printf("\n");
		printf("Sending Time Reply to client \n %s",packet);
		printf("\n");
		
	 	bzero(&odraddr, sizeof(odraddr));
		odraddr.sun_family = AF_LOCAL;
		strcpy(odraddr.sun_path, ODR_PATH);
		socklen_t len=sizeof(struct sockaddr);
		 
		/* getsockname(sockfd, (SA *) &addr, &len);
		printf("bound name = %s, returned len = %d\n", addr.sun_path, len);  */
	
		sendbytes = sendto(sockfd,packet,sizeof(packet),0,(struct sockaddr*) &odraddr,len);
		if(sendbytes < 0)
		{
			printf("sendto function error\n");
		} 
    return 0;

}


 int main(int argc, char **argv)
 {
     int     socketfd,n;
     struct sockaddr_un serveraddr,addr;
	 socklen_t sock_len;
	 char message_rcvd[MAXLINE];
	 char ClientCanonicalIP[INET_ADDRSTRLEN];
	 int port;
	 char buff[MAXLINE+1];
	 char clientvm[MAXLINE],servervm[MAXLINE];
	 time_t ticks; 
	struct in_addr ipv4addr;	
	struct hostent *he;
    socketfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
	int routeflag =0;
	
     unlink(SERVERPATH);
     bzero(&serveraddr, sizeof(serveraddr));
     serveraddr.sun_family = AF_LOCAL;
     strcpy(serveraddr.sun_path,SERVERPATH);

     Bind(socketfd, (SA *) &serveraddr, sizeof(serveraddr));
		
	 
    sock_len = sizeof(addr);
    getsockname(socketfd, (SA *) &addr, &sock_len);

    printf("Name Bounded = %s, returned len = %d  \n", addr.sun_path, sock_len);
	
	gethostname(servervm, sizeof servervm);
	printf("servervm: %s\n", servervm);
	
   while(1){
		printf("waiting for client \n");
        n = msg_rcv(socketfd, message_rcvd, ClientCanonicalIP, &port);
		if(n == 0){
			printf("receive error \n");
		}else if(n == 1){
			printf("Message Received from client\n");
			printf("message received is %s \n",message_rcvd);
			printf("source Canonical IP received is %s \n",ClientCanonicalIP);
			printf("source port is %d \n",port);
			
			
			inet_pton(AF_INET,ClientCanonicalIP, &ipv4addr);
            he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
			strcpy(clientvm,he->h_name);
			
			ticks=time(NULL);
			snprintf(buff, sizeof(buff), "%s\r\n", ctime(&ticks));
			printf("Server at node %s  responding to request from client at node %s\n", servervm, clientvm);
			msg_send(socketfd, ClientCanonicalIP, port, buff, routeflag);
		}
    	
	}
    
    return 0;
 }
