#include "hw_addrs.h"

//constants
#define ODRPATH "/tmp/astayal"
#define SERVERPORT 13854

//global
char DestCanonicalIP[25];

int main(int argc, char **argv)
 {
    int sockfd;
    struct sockaddr_un cliaddr,addr2;
	int fd,svm=0;
	char clientvm[MAXLINE],servervm[MAXLINE];
    struct hostent *he;
     char **ip;
	
    sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);

    bzero(&cliaddr, sizeof(cliaddr));   /* bind an address for us */
    cliaddr.sun_family = AF_LOCAL;

	strcpy(cliaddr.sun_path,"/tmp/ashanaXXXXXX");	/* Copy template */
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
		if( svm<1 && svm>10){
			printf("Please enter vm number between 1-10 \n");
		}
		
		//get client vm 
		gethostname(clientvm, sizeof clientvm);
		printf("clientvm: %s\n", clientvm);
		
		sprintf(servervm,"vm%d",svm);
		fflush(stdout);
		printf("servervm : %s \n",servervm);
		
		
		//char **pptr;
		char msgtosend[10] = "hello";
		int routeflag=0;
		
		//get server canonical IP address
		he = gethostbyname(servervm);
		if (he == NULL) { // do some error checking
			herror("gethostbyname"); 
			exit(1);
		}
		
        ip=he->h_addr_list;
		//printf ("official hostname: %s\n", he->h_name);
		inet_ntop(he->h_addrtype,*ip,DestCanonicalIP,sizeof(DestCanonicalIP));
		//strncpy(DestCanonicalIP, he->h_name, sizeof(DestCanonicalIP) - 1); 
		printf("destination canonical ip : %s \n",DestCanonicalIP);
		printf("client node at %s sending request to server at %s \n",clientvm,servervm);
		
		//sending message to server
		msg_send(sockfd,DestCanonicalIP,SERVERPORT,msgtosend,routeflag);
		printf("\n");
	}
	

   
    exit(0);
 }
