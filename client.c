#include "hw_addrs.h"

//constants
#define ODR_PATH "/tmp/ashana"
#define SERVERPORT 13854

int msg_rcv(int sockfd,char messagercvd[],char CanonicalIP[],int *port)
{

	int rcvdbytes,k=0;
	struct sockaddr_un odraddr;
	char buffer[MAXLINE];
	socklen_t len;
	char msgrcvd[8][MAXLINE];
	memset(buffer,0,sizeof buffer);
	rcvdbytes = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr*) &odraddr, &len);
	
	if(rcvdbytes < 0)
    {
        printf("recvfrom error \n");
    }else{
		char* token = strtok(buffer, "/");
		while(token != NULL) {
        strcpy(msgrcvd[k],token);
        k++;
        token = strtok(NULL, "/");
		}
		strcpy(messagercvd,msgrcvd[7]);
		
	}
    return 0;
}


int msg_send(int sockfd, char ServerCanonicalIP[], int destport, char msgtosend[], int flag)
{
		char packet[MAXLINE];
		struct sockaddr_un odraddr,addr;
		int sendbytes;
		
		snprintf(packet, sizeof(packet), "%d %s %d %s %d", sockfd,ServerCanonicalIP,destport,msgtosend,flag);
		printf("\n");
		printf("Message to be send to server is \n %s",packet);
		printf("\n");
		
	 	bzero(&odraddr, sizeof(odraddr));
		odraddr.sun_family = AF_LOCAL;
		strcpy(odraddr.sun_path, ODR_PATH);
		socklen_t len=sizeof(struct sockaddr);
	
		sendbytes = sendto(sockfd,packet,sizeof(packet),0,(struct sockaddr*) &odraddr,len);
		if(sendbytes < 0)
		{
			printf("sendto function error %d \n", errno);
		} 
    return 0;

}


int main(int argc, char **argv)
 {
    int sockfd;
    struct sockaddr_un cliaddr,addr2;
	int fd,svm=0;
	char clientvm[MAXLINE],servervm[MAXLINE];
    struct hostent *he;
    char **ip;
	char msgtosend[10] = "hello";
	int routeflag;
	char ServerCanonicalIP[25];
	struct timeval tv;
	int rv;
	char message_rcvd[MAXLINE];
	char CanonicalIP[INET_ADDRSTRLEN];
	int port;			
	int nretranmits;		
    sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

    bzero(&cliaddr, sizeof(cliaddr));   /* bind an address for us */
    cliaddr.sun_family = AF_LOCAL;

	strcpy(cliaddr.sun_path,"/tmp/ashXXXXXX");	/* Copy template */
	fd = mkstemp(cliaddr.sun_path);			/* Create and open temp file */
	unlink(cliaddr.sun_path);				/* Remove it */
	close(fd);				/* Close file */
    bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	
	socklen_t len = sizeof(addr2);
	Getsockname(sockfd, (SA *) &addr2, &len);
	printf("bound name = %s, returned len = %d\n", addr2.sun_path, len);

	while(1){
		nretranmits = 5;
		printf("Enter any vm number from 1-10 you want for server \n");
		scanf("%d",&svm);
		printf("You entered vm%d for server \n",svm);
       
        if( svm<1 || svm>10){
            printf("Wrong input entered. Enter vm number between 1-10 \n");
            continue;
        }
        
		
		//get client vm 
		gethostname(clientvm, sizeof clientvm);
		printf("clientvm: %s\n", clientvm);
		
		sprintf(servervm,"vm%d",svm);
		fflush(stdout);
		printf("servervm : %s \n",servervm);

		
		//get server canonical IP address
		he = gethostbyname(servervm);
		if (he == NULL) { // do some error checking
			herror("gethostbyname"); 
			exit(1);
		}
		
        ip=he->h_addr_list;
		inet_ntop(he->h_addrtype,*ip,ServerCanonicalIP,sizeof(ServerCanonicalIP));
		printf("destination canonical ip : %s \n",ServerCanonicalIP);
		printf("client node at %s sending request to server at %s \n",clientvm,servervm);
		routeflag = 0;
		
		//sending message to server
		sendagain:
		msg_send(sockfd,ServerCanonicalIP,SERVERPORT,msgtosend,routeflag);
		printf("\n");
		
		//timer Starts
		while(nretranmits){
				fd_set readfds;
				FD_ZERO(&readfds);
				FD_SET(sockfd,&readfds);
				tv.tv_sec = 5;
				tv.tv_usec=0;
			
				rv =select((sockfd+1),&readfds,NULL,NULL,&tv);
			
				if(rv == -1)
				{
					printf("select error \n");
					continue;
				}
			
				if (rv==0)
				{
					printf("Client Timeout - Retransmitting Request \n");
					routeflag = 1;
					nretranmits--;
					goto sendagain;
				}
			
				if(FD_ISSET(sockfd,&readfds))
				{
					nretranmits = 0;
					printf("Client at node %s, received from server at node %s \n",clientvm,servervm);
					msg_rcv(sockfd, message_rcvd, CanonicalIP, &port);
					printf("Time Reply From Server %s \n",message_rcvd);
				}
			}
	}

    exit(0);
 }
