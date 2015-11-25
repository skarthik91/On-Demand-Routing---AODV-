#include "hw_addrs.h"


//#define MAXLINE 1024
#define ODR_PATH "/tmp/astayal"

int msg_send(int sockfd, char DestCanonicalIP[], int destport, char msgtosend[], int flag)
{
		char packet[MAXLINE];
		struct sockaddr_un odraddr,addr;
		int sendbytes;
		
		snprintf(packet, sizeof(packet), "%d %s %d %s %d", sockfd,DestCanonicalIP,destport,msgtosend,flag);
		printf("\n");
		printf("packet to be sent is \n %s",packet);
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
			printf("API msg_send function couldn't send data. sendto function error. %s \n", strerror(errno));
		}
    
    return 0;
}