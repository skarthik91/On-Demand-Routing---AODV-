/* Our own header for the programs that need hardware address info. */
/*NP Assignment*/

#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>		/* error numbers */
#include <sys/ioctl.h>          /* ioctls */
//#include <net/if.h>             /* generic interface structures */
#include "unp.h"
#include <string.h>
#include <sys/un.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <sys/time.h>
#include <ctype.h>

#include <limits.h>	/* for PATH_MAX */
#include <sys/unistd.h>
#include <netdb.h>

#define	IF_NAME		16	/* same as IFNAMSIZ    in <net/if.h> */
#define	IF_HADDR	 6	/* same as IFHWADDRLEN in <net/if.h> */
#define N_INTERFACES 10
#define	IP_ALIAS  	 1	/* hwa_addr is an alias */

struct hwa_info {
  char    if_name[IF_NAME];	/* interface name, null terminated */
  char    if_haddr[IF_HADDR];	/* hardware address */
  int     if_index;		/* interface index */
  short   ip_alias;		/* 1 if hwa_addr is an alias IP address */
  struct  sockaddr  *ip_addr;	/* IP address */
  struct  hwa_info  *hwa_next;	/* next of these structures */
};

struct Eth_Frame_Header{
	unsigned char     destmac[6];
    unsigned char     sourcemac[6];
    uint16_t packettyped; //protocol; first 3 fields are required for PF_SOCKET
    uint16_t datatype; //0: RREQ; 1:RREP; 2:DATA
    uint16_t broadcastID; //should be set to 0 only for RREP and Data Payload
    uint16_t hopCount;
	uint16_t sourceport;
	uint16_t destport;
    uint8_t  forcediscovery;
    uint8_t  RREPsentflag;
    uint8_t	 payloadlen;//Payload length (excluding FRAME_HEADER size)
	char     DestCanonicalIP[16];
    char     SrcCanonicalIP[16];
	
};

struct Eth_Frame_Payload{
	char payload[1000];
};

struct Routing_Table{
	char destination_IP[16];
	char next_hop[8];
	int  outgoing_index; 
	int  number_hops;
	time_t   time_stamp;
    uint16_t broadcastID[N_INTERFACES];
}routing_table[N_INTERFACES],*ptrroutetable[N_INTERFACES];


/* function prototypes */
struct hwa_info	*get_hw_addrs();
struct hwa_info	*Get_hw_addrs();
void free_hwa_info(struct hwa_info *);
int msg_send(int sockfd, char DestCanonicalIP[], int destport, char msgtosend[], int flag);
