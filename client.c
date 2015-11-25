#include "hw_addrs.h"

//constants
#define ODR_PATH "/tmp/jill"

#define SERVERPORT 13854

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
		 
		getsockname(sockfd, (SA *) &addr, &len);
		printf("bound name = %s, returned len = %d\n", addr.sun_path, len);
	
		sendbytes = sendto(sockfd,packet,sizeof(packet),0,(struct sockaddr*) &odraddr,len);
		if(sendbytes < 0)
		{
			printf("sendto function error\n");
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
	int routeflag=0;
	char ServerCanonicalIP[25];
    sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

    bzero(&cliaddr, sizeof(cliaddr));   /* bind an address for us */
    cliaddr.sun_family = AF_LOCAL;

	strcpy(cliaddr.sun_path,"/tmp/karthikXXXXXX");	/* Copy template */
	fd = mkstemp(cliaddr.sun_path);			/* Create and open temp file */
	unlink(cliaddr.sun_path);				/* Remove it */
	close(fd);				/* Close file */
    bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	
	socklen_t len = sizeof(addr2);
	Getsockname(sockfd, (SA *) &addr2, &len);
	printf("bound name = %s, returned len = %d\n", addr2.sun_path, len);

	while(1){
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
		
		
		//char **pptr;
		
		
		//get server canonical IP address
		he = gethostbyname(servervm);
		if (he == NULL) { // do some error checking
			herror("gethostbyname"); 
			exit(1);
		}
		
        ip=he->h_addr_list;
		//printf ("official hostname: %s\n", he->h_name);
		inet_ntop(he->h_addrtype,*ip,ServerCanonicalIP,sizeof(ServerCanonicalIP));
		//strncpy(DestCanonicalIP, he->h_name, sizeof(DestCanonicalIP) - 1); 
		printf("destination canonical ip : %s \n",ServerCanonicalIP);
		printf("client node at %s sending request to server at %s \n",clientvm,servervm);
		
		//sending message to server
		msg_send(sockfd,ServerCanonicalIP,SERVERPORT,msgtosend,routeflag);
		printf("\n");
	}

    exit(0);
 }
