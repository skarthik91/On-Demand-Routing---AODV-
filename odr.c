#include "hw_addrs.h"



//constants
#define PROTOCOL 562357
#define SOURCEPORT 13855
#define ODR_PATH "/tmp/jill"
#define SERVERPATH "/tmp/jack"

#define ETH_FRAME_LEN 1500
#define RREQ 0
#define RREP 1
#define DATA 2
#define FORWARD 0
#define REVERSE 1
#define DATARECEIVED 10

//#define MAXLINE 1024

//global variables
float stalenessT_out; //in seconds
int table_index;  // Routing table index number

struct clientbuffer{
    
    int sockfd;
    int serverport;
    int flag;
    char message[MAXLINE];
    char destCanonicalIP[MAXLINE];
    char sourceCanonicalIP[MAXLINE];
    int source_index;
    int broadcast;
    int destination_index;
    int odr_index;
    int data;
}client_buffer;




struct Eth_Frame_Header ethframehdr,*ptrethframehdr_send,*ptrethframehdr_rcv;
struct Eth_Frame_Payload ethpayload,*ptrethpayload_send,*ptrethpayload_rcv;
void send_payload(int);
int get_index(char*);
int checkRoute(char [],struct sockaddr_ll ,int);
int check_rreq(struct sockaddr_ll ,void* ,int );

void print_routingtable(int index)
{
    
    int i;
    
    printf("\n \n");
    printf(" Routing Table: \n");
    printf("\n ************************************************************* \n");
    
    printf("\n Destination Canonical IP : %s\n",routing_table[index].destination_IP);
    printf("\n Broadcast ID at source index : %d\n",routing_table[index].broadcastID[index]);
    printf("\n Broadcast ID at destination Index: %d\n",routing_table[client_buffer.destination_index-1].broadcastID[index]);
    printf("\n Number of Hops : %d\n",routing_table[index].number_hops);
    printf("\n Next hop mac Address : ");
    for(i=0;i<6;i++)
    {
        printf("%.2x:",routing_table[index].next_hop[i] & 0xff);
    }
    printf("\n");
    printf("\n Outgoing Index : %d\n",routing_table[index].outgoing_index);
    printf("\n ************************************************************* \n");
}


int add_routing_table(struct sockaddr_ll rcv_pkt_addr,int fwd_rev)
{
    int index;
    int i;
    struct timeval tv;
    //char timebuffer[30];
    
    if(fwd_rev==REVERSE) //Reverse Path Construction
    {
        index=client_buffer.source_index-1;
        strcpy(routing_table[index].destination_IP,ptrethframehdr_rcv->SrcCanonicalIP);
        routing_table[index].broadcastID[index]=ptrethframehdr_rcv->broadcastID; // Storing the broadcast ID at Source entry in the routing table in the Source index array
        routing_table[client_buffer.destination_index-1].broadcastID[index]=ptrethframehdr_rcv->broadcastID; // Storing the broadcast ID at Destination entry in the routing table in the Source index array
        routing_table[index].number_hops=ptrethframehdr_rcv->hopCount;
        
        
        
        for(i=0;i<6;i++)
        {
            routing_table[index].next_hop[i]=rcv_pkt_addr.sll_addr[i] & 0xff;
            
        }
        
        printf("\n");
        routing_table[index].next_hop[7]=0x00;
        routing_table[index].next_hop[6]=0x00;
        
        
        routing_table[index].outgoing_index=rcv_pkt_addr.sll_ifindex;
        gettimeofday(&tv, NULL);
        routing_table[index].time_stamp=tv.tv_sec*(10^6)+tv.tv_usec;
        
        //strftime(timebuffer,30,"%m-%d-%Y  %T.",localtime(&routing_table[index].time_stamp));
        
        
        printf("\n Routing Table - Reverse Path Construction \n");
        print_routingtable(index);
        // printf("\n Time of adding entry to table %ld\n",tv.tv_usec);
        
    }
    
    else if(fwd_rev==FORWARD) //Forward Path Construction
    {
        
        index=client_buffer.destination_index-1;
        strcpy(routing_table[index].destination_IP,ptrethframehdr_rcv->DestCanonicalIP);
        routing_table[index].broadcastID[index]=ptrethframehdr_rcv->broadcastID; // Storing the broadcast ID at Source entry in the routing table in the Source index array
        routing_table[client_buffer.source_index-1].broadcastID[index]=ptrethframehdr_rcv->broadcastID; // Storing the broadcast ID at Destination entry in the routing table in the Source index array
        routing_table[index].number_hops=ptrethframehdr_rcv->hopCount;
        
        
        
        for(i=0;i<6;i++)
        {
            routing_table[index].next_hop[i]=rcv_pkt_addr.sll_addr[i] & 0xff;
            
        }
        
        printf("\n");
        routing_table[index].next_hop[7]=0x00;
        routing_table[index].next_hop[6]=0x00;
        
        
        routing_table[index].outgoing_index=rcv_pkt_addr.sll_ifindex;
        gettimeofday(&tv, NULL);
        routing_table[index].time_stamp=tv.tv_sec;
        
        //strftime(timebuffer,30,"%m-%d-%Y  %T.",localtime(&routing_table[index].time_stamp));
        
        
        printf("\ Routing Table---- Forward Path Construction \n");
        print_routingtable(index);
        // printf("\n Time of adding entry to table %ld\n",tv.tv_usec);
        
    }
    
    else if(fwd_rev==DATA) // FREE RREP
    {
        
        index=client_buffer.source_index-1;
        strcpy(routing_table[index].destination_IP,ptrethframehdr_rcv->SrcCanonicalIP);
   
        
        routing_table[index].number_hops=ptrethframehdr_rcv->hopCount;
        
        
        
        for(i=0;i<6;i++)
        {
            routing_table[index].next_hop[i]=rcv_pkt_addr.sll_addr[i] & 0xff;
            
        }
        
        printf("\n");
        routing_table[index].next_hop[7]=0x00;
        routing_table[index].next_hop[6]=0x00;
        
        
        routing_table[index].outgoing_index=rcv_pkt_addr.sll_ifindex;
        gettimeofday(&tv, NULL);
        routing_table[index].time_stamp=tv.tv_sec;
        
        //strftime(timebuffer,30,"%m-%d-%Y  %T.",localtime(&routing_table[index].time_stamp));
        
        
        printf("\n Routing Table - Reverse Path Construction \n");
        print_routingtable(index);
        // printf("\n Time of adding entry to table %ld\n",tv.tv_usec);
        
        
        
    }
    
    return 0;
    
}


int prhwaddrs()
{
    struct hwa_info	*hwa, *hwahead;
    struct sockaddr	*sa;
    char   *ptr;
    int    i, prflag;
    int fl;
    
    for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    {
        // ignoring lo and eth0 interfaces
        if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0)
        {
            
            printf("%s :%s", hwa->if_name, ((hwa->ip_alias) == IP_ALIAS) ? " (alias)\n" : "\n");
            
            if ( (sa = hwa->ip_addr) != NULL)
                printf("IP addr = %s\n", Sock_ntop_host(sa, sizeof(*sa)));
            
            prflag = 0;
            i = 0;
            do {
                if (hwa->if_haddr[i] != '\0')
                {
                    prflag = 1;
                    break;
                }
            } while (++i < IF_HADDR);
            if (prflag)
            {
                printf("HW addr = ");
                ptr = hwa->if_haddr;
                i = IF_HADDR;
                fl=0;
                do{
                    if((*ptr++ & 0xff)!=0x00)
                    {
                        fl=1;
                    }
                } while (--i > 0);
                
                ptr = hwa->if_haddr;
                i = IF_HADDR;
                if(fl==1)
                {
                    do{
                        printf("%.2x%s", *ptr++ & 0xff, (i == 1) ? " " : ":");
                    } while (--i > 0);
                }
                
                else
                {
                    printf("\n ");
                }
                
            }
            printf("\n Interface index = %d\n\n", hwa->if_index);
        }
    }
    free_hwa_info(hwahead);
}

int initialRREQ(int packet_socket,int skip_index,int forcediscovery)
{
    int j;
    struct hwa_info	*hwa, *hwahead;
    char   *ptr;
    int    i;
    /*target address*/
    struct sockaddr_ll socket_address;
    unsigned char src_mac[6];
    
    for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    {
        if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0 && hwa->if_index != skip_index)
        {
            
            ptr = hwa->if_haddr;
            i = IF_HADDR;
            j=0;
            /*Loading source Mac Address*/
            do{
                src_mac[j] = *ptr++ & 0xff;
            } while (--i > 0 && j++ < 5);
            
            
            
            /*buffer for ethernet frame*/
            void* buffer = (void*)malloc(ETH_FRAME_LEN);
            
            /*pointer to ethenet header*/
            //unsigned char* etherhead = buffer;
            ptrethframehdr_send = buffer;
            
            /*userdata in ethernet frame*/
            ptrethpayload_send = (buffer + sizeof(struct Eth_Frame_Header));
            
            //Loading values to ethernet header
            ptrethframehdr_send->packettyped=htons(PROTOCOL);
            ptrethframehdr_send->datatype=RREQ;
            ptrethframehdr_send->broadcastID=client_buffer.broadcast;
            ptrethframehdr_send->sourceport = SOURCEPORT;
			ptrethframehdr_send->destport = client_buffer.serverport;
            ptrethframehdr_send->hopCount=1;
            ptrethframehdr_send->forcediscovery=client_buffer.flag;
            
            ptrethframehdr_send->RREPsentflag=0;
            ptrethframehdr_send->payloadlen=100;
            strcpy(ptrethframehdr_send->DestCanonicalIP,client_buffer.destCanonicalIP);
            strcpy(ptrethframehdr_send->SrcCanonicalIP,client_buffer.sourceCanonicalIP);
            
            int send_result = 0;
            
            
            /*other host MAC address*/
            unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
            
            
            /*RAW communication*/
            socket_address.sll_family   = PF_PACKET;
            /*we don't use a protocoll above ethernet layer
             ->just use anything here*/
            socket_address.sll_protocol = htons(PROTOCOL);
            
            /*index of the network device
             see full code later how to retrieve it*/
            socket_address.sll_ifindex  = hwa->if_index;
            
            /*ARP hardware identifier is ethernet*/
            socket_address.sll_hatype   = ARPHRD_ETHER;
            
            /*target is another host*/
            socket_address.sll_pkttype  = PACKET_OTHERHOST;
            
            /*address length*/
            socket_address.sll_halen    = ETH_ALEN;
            /*MAC - begin*/
            socket_address.sll_addr[0]  = 0xff;
            socket_address.sll_addr[1]  = 0xff;
            socket_address.sll_addr[2]  = 0xff;
            socket_address.sll_addr[3]  = 0xff;
            socket_address.sll_addr[4]  = 0xff;
            socket_address.sll_addr[5]  = 0xff;
            /*MAC - end*/
            socket_address.sll_addr[6]  = 0x00;/*not used*/
            socket_address.sll_addr[7]  = 0x00;/*not used*/
            
            
            /*set the frame header*/
            memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
            memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
            
            strcpy(ptrethpayload_send->payload,client_buffer.message);
            
            printf("sending intial RREQ buffer : %s\n",ptrethpayload_send->payload);
            /*send the packet*/
            send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 (struct sockaddr*)&socket_address, sizeof(socket_address));
            if (send_result == -1) {
                printf("Sending error : %d",errno);
                exit(1);
                
            }
            
            
            
            
        }
    }
    return 0;

}

int floodRREQ(void* buffer, int packet_socket,int skip_index,int RREPsentflag)
{
    
    int j;
    struct hwa_info	*hwa, *hwahead;
    char   *ptr;
    int    i;
    /*target address*/
    struct sockaddr_ll socket_address;
    unsigned char src_mac[6];
    
    for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    {
        if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0 && hwa->if_index != skip_index)
        {
            
            ptr = hwa->if_haddr;
            i = IF_HADDR;
            j=0;
            /*Loading source Mac Address*/
            do{
                src_mac[j] = *ptr++ & 0xff;
            } while (--i > 0 && j++ < 5);
            
            /*pointer to ethenet header*/
            //unsigned char* etherhead = buffer;
            ptrethframehdr_send = buffer;
            
            /*userdata in ethernet frame*/
            ptrethpayload_send = (buffer + sizeof(struct Eth_Frame_Header));
            
            //Loading values to ethernet header
            
            ptrethframehdr_send->hopCount=ptrethframehdr_rcv->hopCount;
            ptrethframehdr_send->hopCount++;
            ptrethframehdr_send->RREPsentflag=RREPsentflag;
            ptrethframehdr_send->forcediscovery=ptrethframehdr_rcv->forcediscovery;
            int send_result = 0;
            
            /*other host MAC address*/
            unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
            
            /*RAW communication*/
            socket_address.sll_family   = PF_PACKET;
            /*we don't use a protocoll above ethernet layer
             ->just use anything here*/
            socket_address.sll_protocol = htons(PROTOCOL);
            
            /*index of the network device
             see full code later how to retrieve it*/
            socket_address.sll_ifindex  = hwa->if_index;
            
            /*ARP hardware identifier is ethernet*/
            socket_address.sll_hatype   = ARPHRD_ETHER;
            
            /*target is another host*/
            socket_address.sll_pkttype  = PACKET_OTHERHOST;
            
            /*address length*/
            socket_address.sll_halen    = ETH_ALEN;
            /*MAC - begin*/
            socket_address.sll_addr[0]  = 0xff;
            socket_address.sll_addr[1]  = 0xff;
            socket_address.sll_addr[2]  = 0xff;
            socket_address.sll_addr[3]  = 0xff;
            socket_address.sll_addr[4]  = 0xff;
            socket_address.sll_addr[5]  = 0xff;
            /*MAC - end*/
            socket_address.sll_addr[6]  = 0x00;/*not used*/
            socket_address.sll_addr[7]  = 0x00;/*not used*/
            
            
            /*set the frame header*/
            memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
            memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
            
            //strcpy(ptrethpayload_send->payload,ptrethpayload_rcv->payload);
            
            printf("sending flooding RREQ buffer : %s\n",ptrethpayload_send->payload);
            /*send the packet*/
            send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 (struct sockaddr*)&socket_address, sizeof(socket_address));
            if (send_result == -1) {
                printf("Sending error : %d",errno);
                exit(1);
                
            }
            
        }
    }
    return 0;
}

void split_buffer(char unixbuffer[])
{
    char msgrcvd[5][MAXLINE];
    int k = 0;
    char* token = strtok(unixbuffer, " ");
    struct hostent *he;
    struct in_addr ipv4addr;
    char vm_string[2];
    char serverodrvm[MAXLINE];
    
    
    while(token != NULL) {
        strcpy(msgrcvd[k],token);
        k++;
        token = strtok(NULL, " ");
    }
    client_buffer.sockfd = atoi(msgrcvd[0]);
    strcpy(client_buffer.destCanonicalIP ,msgrcvd[1]);
    client_buffer.serverport = atoi(msgrcvd[2]);
    strcpy(client_buffer.message, msgrcvd[3]);
    client_buffer.flag = atoi(msgrcvd[4]);
    
    
    inet_pton(AF_INET,msgrcvd[1], &ipv4addr);
    
   
    he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
    
    printf("Server name: %s\n",he->h_name);
   
    client_buffer.destination_index=get_index(he->h_name);
    
    printf("\n Server index is %d \n",client_buffer.destination_index);
    
    
}
int get_index(char* vm_string)
{
    
    if(strcmp(vm_string,"vm1")==0)
    {
        return 1;
        
    }
    
    else if(strcmp(vm_string,"vm2")==0)
    {
        return 2;
        
    }
    
    else if(strcmp(vm_string,"vm3")==0)
    {
        return 3;
        
    }
    
    
    else if(strcmp(vm_string,"vm4")==0)
    {
        return 4;
        
    }
    
    else if(strcmp(vm_string,"vm5")==0)
    {
        return 5;
        
    }
    
    else if(strcmp(vm_string,"vm6")==0)
    {
        return 6;
        
    }
    
    else if(strcmp(vm_string,"vm7")==0)
    {
        return 7;
        
    }
    
    else if(strcmp(vm_string,"vm8")==0)
    {
        return 8;
        
    }
    
    else if(strcmp(vm_string,"vm9")==0)
    {
        return 9;
        
    }
    
    else if(strcmp(vm_string,"vm10")==0)
    {
        return 10;
        
    }
    
    else {
        
        printf("\n Error in resolving index \n");
        return -1;
    }
    return -1;
    
    
}

int check_rreq(struct sockaddr_ll rcv_pkt_addr,void* buffer,int packet_socket)
{
    
    int skip_index=rcv_pkt_addr.sll_ifindex;
    
    if(ptrethframehdr_rcv->forcediscovery==1)
    {
        printf("\n RREQ arrived with forced discovery bit set\n");
        if(client_buffer.destination_index==client_buffer.odr_index)
        {
            printf(" I am the destination. Sending RREP in the forced discovery mode ");
            add_routing_table(rcv_pkt_addr,REVERSE);
            sendrrep(packet_socket);
            
            printf("\n Propogating the RREQ with RREP sent flag set");
            floodRREQ(buffer,packet_socket,skip_index,1);
            return 0;
            
        }
        
        else
        {
            printf("\nPropogating RREQ in the forced discovery mode\n");
            add_routing_table(rcv_pkt_addr,REVERSE);
            floodRREQ(buffer,packet_socket,skip_index, ptrethframehdr_rcv->RREPsentflag);
            
                
        }
        
        
        
        
    }
    
 else if(ptrethframehdr_rcv->RREPsentflag==1)
    {
        printf("\n RREQ received with RREP already sent flag at ODR vm %d set high. Client vm%d for destination vm%d Updating routing table and discarding RREQ \n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
        add_routing_table(rcv_pkt_addr,REVERSE);
        return 0;
    }
    //Checking for broadcast ID
   else if(ptrethframehdr_rcv->broadcastID > routing_table[client_buffer.destination_index-1].broadcastID[client_buffer.source_index-1] || routing_table[client_buffer.destination_index-1].broadcastID[client_buffer.source_index-1]==0)
    {
        printf("\n\n New RREQ received -  Updating routing table \n\n");
        add_routing_table(rcv_pkt_addr,REVERSE);
        
        
        if(checkRoute(ptrethframehdr_rcv->DestCanonicalIP,rcv_pkt_addr,packet_socket)==1) // Route to destination found . Add routing table and send RREP
        {
            printf("\n Route to Destination found at ODR vm%d. Sending RREP to client vm%d for destination vm%d \n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
            add_routing_table(rcv_pkt_addr,REVERSE);
            sendrrep(packet_socket);
            
            printf("\n Propogating the RREQ with RREP sent flag set");
            floodRREQ(buffer,packet_socket,skip_index,1);
            return 1;
        }
        
        
        else if(checkRoute(ptrethframehdr_rcv->DestCanonicalIP,rcv_pkt_addr,packet_socket)==0)
        {
            printf("\n No Route found to destination. Flooding RREQ at ODR vm%d for client vm%d and destination vm%d \n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
            add_routing_table(rcv_pkt_addr,REVERSE);
            floodRREQ(buffer,packet_socket,skip_index, ptrethframehdr_rcv->RREPsentflag);
            
            return 0;
        }
        
    }
    
    else if(ptrethframehdr_rcv->broadcastID <= routing_table[client_buffer.destination_index-1].broadcastID[client_buffer.source_index-1])
    {
        
        printf("\n Out of place broadcast ID arrived. Checking if it has lower hop count at vm%d from client vm%d at destination vm%d \n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
        
        
        //Check for hop count condition for effiecent path
        if(ptrethframehdr_rcv->hopCount<routing_table[client_buffer.source_index-1].number_hops || routing_table[client_buffer.source_index-1].number_hops==0)
        {
            printf("\n RREQ has out of place broadcast ID but better hopcount - Updating routing table \n");
            if(checkRoute(ptrethframehdr_rcv->DestCanonicalIP,rcv_pkt_addr,packet_socket)==1) // Route to destination found . Add routing table and send RREP
            {
                printf("\n Route to Destination found at ODR vm%d. Sending RREP to client vm%d for destination vm%d \n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
                add_routing_table(rcv_pkt_addr,REVERSE);
                sendrrep(packet_socket);
                printf("\n Propogating the RREQ with RREP sent flag set");
                floodRREQ(buffer,packet_socket,skip_index,1);
                return 1;
            }
            
            
            else if(checkRoute(ptrethframehdr_rcv->DestCanonicalIP,rcv_pkt_addr,packet_socket)==0)
            {
                printf("\n No Route found to destination. Flooding RREQ at ODR vm%d for client vm%d and destination vm%d \n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
                add_routing_table(rcv_pkt_addr,REVERSE);
                floodRREQ(buffer,packet_socket,skip_index,ptrethframehdr_rcv->RREPsentflag);
                
                return 0;
            }
            
            
        }
        
        else
        {
            printf("\n Inefficient path and out of place broadcast ID packet  RREQ arrived. Discarding Packet \n");
            return 0;
        }
        
        
    }
    
    
}



//Checking for a path
int checkRoute(char destIP[],struct sockaddr_ll rcv_pkt_addr,int packet_socket)
{
    int i;
    char odrvm[5];
    char vm_string[2];
    
    memset(odrvm,0,sizeof(odrvm));
    gethostname(odrvm, sizeof odrvm);
    printf("odrvm: %s\n", odrvm);
    
    
    //Loading ODR index
    client_buffer.odr_index=get_index(odrvm);
    printf("\n ODR index is %d \n",client_buffer.odr_index);
    
    
    //Check if the ODR itself is the server
    
    if(client_buffer.odr_index==client_buffer.destination_index)
    {
        add_routing_table(rcv_pkt_addr,REVERSE);
        //Updating Destination Routing Table.
        
        printf("\n I, vm%d am the destination. Sending RREP to vm%d.  \n",client_buffer.odr_index,client_buffer.source_index); //$$$$$$$$$$ To complete this
        return 1;
    }
    
    for(i=0;i<N_INTERFACES;i++)
    {
        if(strcmp(destIP,routing_table[i].destination_IP) == 0)
        {
            add_routing_table(rcv_pkt_addr,REVERSE);
            table_index=i;  //Setting table index
            printf("\n Route to destination found in intermediate ODR - sending RREP\n");
            return 1;
        }
    }
    
    return 0;
    
    
}



int relay_rrep(struct sockaddr_ll rcv_pkt_addr,int packet_socket)
{
    int j;
    struct hwa_info	*hwa, *hwahead;
    char   *ptr;
    int    i;
    /*target address*/
    struct sockaddr_ll socket_address;
    unsigned char src_mac[6];
    unsigned char dest_mac[6];
    
    for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    {
        if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0 && hwa->if_index == routing_table[client_buffer.source_index-1].outgoing_index)
        {
            
            ptr = hwa->if_haddr;
            i = IF_HADDR;
            j=0;
            /*Loading source Mac Address*/
            do{
                src_mac[j] = *ptr++ & 0xff;
            } while (--i > 0 && j++ < 5);
            
            
            
            /*buffer for ethernet frame*/
            void* buffer = (void*)malloc(ETH_FRAME_LEN);
            
            /*pointer to ethenet header*/
            //unsigned char* etherhead = buffer;
            ptrethframehdr_send = buffer;
            
            /*userdata in ethernet frame*/
            ptrethpayload_send = (buffer + sizeof(struct Eth_Frame_Header));
            
            //Loading values to ethernet header
            ptrethframehdr_send->packettyped=htons(PROTOCOL);
            ptrethframehdr_send->datatype=RREP;
            ptrethframehdr_send->broadcastID=0;
			ptrethframehdr_send->sourceport = SOURCEPORT;
			ptrethframehdr_send->destport = client_buffer.serverport;
            ptrethframehdr_send->RREPsentflag=0;
            ptrethframehdr_send->hopCount=ptrethframehdr_rcv->hopCount;
            ptrethframehdr_send->hopCount++;
            ptrethframehdr_send->forcediscovery=0;
            ptrethframehdr_send->payloadlen=100;
            strcpy(ptrethframehdr_send->DestCanonicalIP,client_buffer.destCanonicalIP);
            strcpy(ptrethframehdr_send->SrcCanonicalIP,client_buffer.sourceCanonicalIP);
            
            int send_result = 0;
            
            
            /*other host MAC address*/
            
            dest_mac[0]  = routing_table[client_buffer.source_index-1].next_hop[0] & 0xff;
            dest_mac[1]  = routing_table[client_buffer.source_index-1].next_hop[1] & 0xff;
            dest_mac[2]  = routing_table[client_buffer.source_index-1].next_hop[2] & 0xff;
            dest_mac[3]  = routing_table[client_buffer.source_index-1].next_hop[3] & 0xff;
            dest_mac[4]  = routing_table[client_buffer.source_index-1].next_hop[4] & 0xff;
            dest_mac[5]  = routing_table[client_buffer.source_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            
            /*RAW communication*/
            socket_address.sll_family   = PF_PACKET;
            /*we don't use a protocoll above ethernet layer
             ->just use anything here*/
            socket_address.sll_protocol = htons(PROTOCOL);
            
            /*index of the network device
             see full code later how to retrieve it*/
            socket_address.sll_ifindex  = hwa->if_index;
            
            /*ARP hardware identifier is ethernet*/
            socket_address.sll_hatype   = ARPHRD_ETHER;
            
            /*target is another host*/
            socket_address.sll_pkttype  = PACKET_OTHERHOST;
            
            /*address length*/
            socket_address.sll_halen    = ETH_ALEN;
            
            
            
            /*Loading - Next Hop Address*/
            socket_address.sll_addr[0]  = routing_table[client_buffer.source_index-1].next_hop[0] & 0xff;
            socket_address.sll_addr[1]  = routing_table[client_buffer.source_index-1].next_hop[1] & 0xff;
            socket_address.sll_addr[2]  = routing_table[client_buffer.source_index-1].next_hop[2] & 0xff;
            socket_address.sll_addr[3]  = routing_table[client_buffer.source_index-1].next_hop[3] & 0xff;
            socket_address.sll_addr[4]  = routing_table[client_buffer.source_index-1].next_hop[4] & 0xff;
            socket_address.sll_addr[5]  = routing_table[client_buffer.source_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            socket_address.sll_addr[6]  = 0x00;/*not used*/
            socket_address.sll_addr[7]  = 0x00;/*not used*/
            
            
            /*set the frame header*/
            memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
            memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
            
            strcpy(ptrethpayload_send->payload,client_buffer.message);
            add_routing_table(rcv_pkt_addr,FORWARD);
            printf("sending relay for client index vm%d from destination index vm%d at odr index vm %d RREP buffer : %s\n",client_buffer.source_index,client_buffer.destination_index,client_buffer.odr_index,ptrethpayload_send->payload);
            /*send the packet*/
            send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 (struct sockaddr*)&socket_address, sizeof(socket_address));
            if (send_result == -1) {
                printf("Sending error : %d",errno);
                exit(1);
                
            }
            
            
            
            
        }
    }
    
    
    
    
    return 0;
    
}

void sendrrep(int packet_socket)
{
    
    int j;
    struct hwa_info	*hwa, *hwahead;
    char   *ptr;
    int    i;
    /*target address*/
    struct sockaddr_ll socket_address;
    unsigned char src_mac[6];
    unsigned char dest_mac[6];
    
    for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    {
        
        if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0 && hwa->if_index == routing_table[client_buffer.source_index-1].outgoing_index)
        {
            
            ptr = hwa->if_haddr;
            i = IF_HADDR;
            j=0;
            /*Loading source Mac Address*/
            do{
                src_mac[j] = *ptr++ & 0xff;
            } while (--i > 0 && j++ < 5);
            
            
            
            /*buffer for ethernet frame*/
            void* buffer = (void*)malloc(ETH_FRAME_LEN);
            
            /*pointer to ethenet header*/
            //unsigned char* etherhead = buffer;
            ptrethframehdr_send = buffer;
            
            /*userdata in ethernet frame*/
            ptrethpayload_send = (buffer + sizeof(struct Eth_Frame_Header));
            
            //Loading values to ethernet header
            ptrethframehdr_send->packettyped=htons(PROTOCOL);
            ptrethframehdr_send->datatype=RREP;
            ptrethframehdr_send->broadcastID=0;
			ptrethframehdr_send->sourceport = SOURCEPORT;
			ptrethframehdr_send->destport = client_buffer.serverport;
            ptrethframehdr_send->RREPsentflag=0;
            ptrethframehdr_send->hopCount=1;
            ptrethframehdr_send->forcediscovery=0;
            ptrethframehdr_send->payloadlen=100;
            strcpy(ptrethframehdr_send->DestCanonicalIP,client_buffer.destCanonicalIP);
            strcpy(ptrethframehdr_send->SrcCanonicalIP,client_buffer.sourceCanonicalIP);
            
            int send_result = 0;
            
            
            /*other host MAC address*/
            
            dest_mac[0]  = routing_table[client_buffer.source_index-1].next_hop[0] & 0xff;
            dest_mac[1]  = routing_table[client_buffer.source_index-1].next_hop[1] & 0xff;
            dest_mac[2]  = routing_table[client_buffer.source_index-1].next_hop[2] & 0xff;
            dest_mac[3]  = routing_table[client_buffer.source_index-1].next_hop[3] & 0xff;
            dest_mac[4]  = routing_table[client_buffer.source_index-1].next_hop[4] & 0xff;
            dest_mac[5]  = routing_table[client_buffer.source_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            
            /*RAW communication*/
            socket_address.sll_family   = PF_PACKET;
            /*we don't use a protocoll above ethernet layer
             ->just use anything here*/
            socket_address.sll_protocol = htons(PROTOCOL);
            
            /*index of the network device
             see full code later how to retrieve it*/
            socket_address.sll_ifindex  = hwa->if_index;
            
            /*ARP hardware identifier is ethernet*/
            socket_address.sll_hatype   = ARPHRD_ETHER;
            
            /*target is another host*/
            socket_address.sll_pkttype  = PACKET_OTHERHOST;
            
            /*address length*/
            socket_address.sll_halen    = ETH_ALEN;
            
            
            
            /*Loading - Next Hop Address*/
            socket_address.sll_addr[0]  = routing_table[client_buffer.source_index-1].next_hop[0] & 0xff;
            socket_address.sll_addr[1]  = routing_table[client_buffer.source_index-1].next_hop[1] & 0xff;
            socket_address.sll_addr[2]  = routing_table[client_buffer.source_index-1].next_hop[2] & 0xff;
            socket_address.sll_addr[3]  = routing_table[client_buffer.source_index-1].next_hop[3] & 0xff;
            socket_address.sll_addr[4]  = routing_table[client_buffer.source_index-1].next_hop[4] & 0xff;
            socket_address.sll_addr[5]  = routing_table[client_buffer.source_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            socket_address.sll_addr[6]  = 0x00;/*not used*/
            socket_address.sll_addr[7]  = 0x00;/*not used*/
            
            
            /*set the frame header*/
            memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
            memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
            
            strcpy(ptrethpayload_send->payload,client_buffer.message);
            
            printf("sending intial RREP buffer : %s\n",ptrethpayload_send->payload);
            /*send the packet*/
            send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 (struct sockaddr*)&socket_address, sizeof(socket_address));
            if (send_result == -1) {
                printf("Sending error : %d",errno);
                exit(1);
                
            }
            
            
            
            
        }
    }
    return 0;
}

void print_ethernetframe(int length)
{
   
    
    printf("\n \n");
    printf("\nPrinting Ethernet Frame\n");
    printf("********************************************************************************* \n");
    printf("\n received  packet type: %d\n",ptrethframehdr_rcv->packettyped);
    printf("\n received data type: %d\n",ptrethframehdr_rcv->datatype);
    printf("\n received broadcast id : %d\n",ptrethframehdr_rcv->broadcastID);
    printf("\n received hop count : %d\n",ptrethframehdr_rcv->hopCount);
    printf("\n received flag : %d\n",ptrethframehdr_rcv->forcediscovery);
    printf("\n received destination canonical IP : %s\n",ptrethframehdr_rcv->DestCanonicalIP);
    printf("\n received source canonical IP : %s\n",ptrethframehdr_rcv->SrcCanonicalIP);
    printf("************************************************************************************ \n");
    printf("\n \n");
    
    
}

int relay_payload(struct sockaddr_ll rcv_pkt_addr,int packet_socket)
{
    int j;
    struct hwa_info	*hwa, *hwahead;
    char   *ptr;
    int    i;
    /*target address*/
    struct sockaddr_ll socket_address;
    unsigned char src_mac[6];
    unsigned char dest_mac[6];
    
    for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    {
        if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0 && hwa->if_index == routing_table[client_buffer.destination_index-1].outgoing_index)
        {
            
            ptr = hwa->if_haddr;
            i = IF_HADDR;
            j=0;
            /*Loading source Mac Address*/
            do{
                src_mac[j] = *ptr++ & 0xff;
            } while (--i > 0 && j++ < 5);
            
            
            
            /*buffer for ethernet frame*/
            void* buffer = (void*)malloc(ETH_FRAME_LEN);
            
            /*pointer to ethenet header*/
            //unsigned char* etherhead = buffer;
            ptrethframehdr_send = buffer;
            
            /*userdata in ethernet frame*/
            ptrethpayload_send = (buffer + sizeof(struct Eth_Frame_Header));
            
            //Loading values to ethernet header
            ptrethframehdr_send->packettyped=htons(PROTOCOL);
            ptrethframehdr_send->datatype=DATA;
            ptrethframehdr_send->broadcastID=0;
			ptrethframehdr_send->sourceport = SOURCEPORT;
			ptrethframehdr_send->destport = client_buffer.serverport;
            ptrethframehdr_send->RREPsentflag=0;
            ptrethframehdr_send->hopCount=ptrethframehdr_rcv->hopCount;
            ptrethframehdr_send->hopCount++;
            ptrethframehdr_send->forcediscovery=0;
            ptrethframehdr_send->payloadlen=100;
            strcpy(ptrethframehdr_send->DestCanonicalIP,client_buffer.destCanonicalIP);
            strcpy(ptrethframehdr_send->SrcCanonicalIP,client_buffer.sourceCanonicalIP);
            
            int send_result = 0;
            
            
            /*other host MAC address*/
            
            dest_mac[0]  = routing_table[client_buffer.destination_index-1].next_hop[0] & 0xff;
            dest_mac[1]  = routing_table[client_buffer.destination_index-1].next_hop[1] & 0xff;
            dest_mac[2]  = routing_table[client_buffer.destination_index-1].next_hop[2] & 0xff;
            dest_mac[3]  = routing_table[client_buffer.destination_index-1].next_hop[3] & 0xff;
            dest_mac[4]  = routing_table[client_buffer.destination_index-1].next_hop[4] & 0xff;
            dest_mac[5]  = routing_table[client_buffer.destination_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            
            /*RAW communication*/
            socket_address.sll_family   = PF_PACKET;
            /*we don't use a protocoll above ethernet layer
             ->just use anything here*/
            socket_address.sll_protocol = htons(PROTOCOL);
            
            /*index of the network device
             see full code later how to retrieve it*/
            socket_address.sll_ifindex  = hwa->if_index;
            
            /*ARP hardware identifier is ethernet*/
            socket_address.sll_hatype   = ARPHRD_ETHER;
            
            /*target is another host*/
            socket_address.sll_pkttype  = PACKET_OTHERHOST;
            
            /*address length*/
            socket_address.sll_halen    = ETH_ALEN;
            
            
            
            /*Loading - Next Hop Address*/
            socket_address.sll_addr[0]  = routing_table[client_buffer.destination_index-1].next_hop[0] & 0xff;
            socket_address.sll_addr[1]  = routing_table[client_buffer.destination_index-1].next_hop[1] & 0xff;
            socket_address.sll_addr[2]  = routing_table[client_buffer.destination_index-1].next_hop[2] & 0xff;
            socket_address.sll_addr[3]  = routing_table[client_buffer.destination_index-1].next_hop[3] & 0xff;
            socket_address.sll_addr[4]  = routing_table[client_buffer.destination_index-1].next_hop[4] & 0xff;
            socket_address.sll_addr[5]  = routing_table[client_buffer.destination_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            socket_address.sll_addr[6]  = 0x00;/*not used*/
            socket_address.sll_addr[7]  = 0x00;/*not used*/
            
            
            /*set the frame header*/
            memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
            memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
            
            strcpy(ptrethpayload_send->payload,ptrethpayload_rcv->payload);
            printf("\n\n\n$$$$$ Relaying Data Packet$$$$$$$$\n");
            printf("Relaying the data packet : %s\n",ptrethpayload_send->payload);
            /*send the packet*/
            send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 (struct sockaddr*)&socket_address, sizeof(socket_address));
            if (send_result == -1) {
                printf("Sending error : %d",errno);
                exit(1);
                
            }
            
            
            
            
        }
    }
    
    printf("\n Updating Routing table for free RREP \n");
    add_routing_table(rcv_pkt_addr,REVERSE);
    
    
    return 0;
    
}

void send_payload(int packet_socket)
{
    int j;
    struct hwa_info	*hwa, *hwahead;
    char   *ptr;
    int    i;
    /*target address*/
    struct sockaddr_ll socket_address;
    unsigned char src_mac[6];
    unsigned char dest_mac[6];
    
    for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    {
        
        if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0 && hwa->if_index == routing_table[client_buffer.destination_index-1].outgoing_index)
        {
            
            ptr = hwa->if_haddr;
            i = IF_HADDR;
            j=0;
            /*Loading source Mac Address*/
            do{
                src_mac[j] = *ptr++ & 0xff;
            } while (--i > 0 && j++ < 5);
            
            
            
            /*buffer for ethernet frame*/
            void* buffer = (void*)malloc(ETH_FRAME_LEN);
            
            /*pointer to ethenet header*/
            //unsigned char* etherhead = buffer;
            ptrethframehdr_send = buffer;
            
            /*userdata in ethernet frame*/
            ptrethpayload_send = (buffer + sizeof(struct Eth_Frame_Header));
            
            //Loading values to ethernet header
            ptrethframehdr_send->packettyped=htons(PROTOCOL);
            ptrethframehdr_send->datatype=DATA;
            ptrethframehdr_send->broadcastID=0;
			ptrethframehdr_send->sourceport = SOURCEPORT;
			ptrethframehdr_send->destport = client_buffer.serverport;
            ptrethframehdr_send->RREPsentflag=0;
            ptrethframehdr_send->hopCount=1;
            ptrethframehdr_send->forcediscovery=0;
            ptrethframehdr_send->payloadlen=100;
            strcpy(ptrethframehdr_send->DestCanonicalIP,client_buffer.destCanonicalIP);
            strcpy(ptrethframehdr_send->SrcCanonicalIP,client_buffer.sourceCanonicalIP);
            
            int send_result = 0;
            
            
            /*other host MAC address*/
            
            dest_mac[0]  = routing_table[client_buffer.destination_index-1].next_hop[0] & 0xff;
            dest_mac[1]  = routing_table[client_buffer.destination_index-1].next_hop[1] & 0xff;
            dest_mac[2]  = routing_table[client_buffer.destination_index-1].next_hop[2] & 0xff;
            dest_mac[3]  = routing_table[client_buffer.destination_index-1].next_hop[3] & 0xff;
            dest_mac[4]  = routing_table[client_buffer.destination_index-1].next_hop[4] & 0xff;
            dest_mac[5]  = routing_table[client_buffer.destination_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            
            /*RAW communication*/
            socket_address.sll_family   = PF_PACKET;
            /*we don't use a protocoll above ethernet layer
             ->just use anything here*/
            socket_address.sll_protocol = htons(PROTOCOL);
            
            /*index of the network device
             see full code later how to retrieve it*/
            socket_address.sll_ifindex  = hwa->if_index;
            
            /*ARP hardware identifier is ethernet*/
            socket_address.sll_hatype   = ARPHRD_ETHER;
            
            /*target is another host*/
            socket_address.sll_pkttype  = PACKET_OTHERHOST;
            
            /*address length*/
            socket_address.sll_halen    = ETH_ALEN;
            
            
            
            /*Loading - Next Hop Address*/
            socket_address.sll_addr[0]  = routing_table[client_buffer.destination_index-1].next_hop[0] & 0xff;
            socket_address.sll_addr[1]  = routing_table[client_buffer.destination_index-1].next_hop[1] & 0xff;
            socket_address.sll_addr[2]  = routing_table[client_buffer.destination_index-1].next_hop[2] & 0xff;
            socket_address.sll_addr[3]  = routing_table[client_buffer.destination_index-1].next_hop[3] & 0xff;
            socket_address.sll_addr[4]  = routing_table[client_buffer.destination_index-1].next_hop[4] & 0xff;
            socket_address.sll_addr[5]  = routing_table[client_buffer.destination_index-1].next_hop[5] & 0xff;
            /*MAC - end*/
            socket_address.sll_addr[6]  = 0x00;/*not used*/
            socket_address.sll_addr[7]  = 0x00;/*not used*/
            
            
            /*set the frame header*/
            memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
            memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
            
            strcpy(ptrethpayload_send->payload,client_buffer.message);
            printf("$$$$$\n\n Sending Initial Buffer\n\n");
            printf("\n Sending intial Data : %s \n",ptrethpayload_send->payload);
            /*send the packet*/
            send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 (struct sockaddr*)&socket_address, sizeof(socket_address));
            if (send_result == -1) {
                printf("\n Sending error : %d \n",errno);
                exit(1);
                
            }
            
            
            
            
        }
    }
    return 0;
}

void senddata_server(int unixdomain_socket,char serverpayload[])
{
	
	struct sockaddr_un servaddr;
	
	int sendbytes;
    printf("\n Payload to be sent to Server %s\n",serverpayload);
	
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, SERVERPATH);
	
	socklen_t len = sizeof(struct sockaddr);
	sendbytes = sendto(unixdomain_socket,serverpayload,MAXLINE,0,(struct sockaddr*)&servaddr,len);
	if(sendbytes <0)
	{
		printf("sendto server error %d \n",errno);
	}
  
}


// void senddata_client(char unixbuffer[], int packet_socket, char serverodrvm[]){
	
	// int j;
    // struct hwa_info	*hwa, *hwahead;
    // char   *ptr;
    // int    i;
    // /*target address*/
    // struct sockaddr_ll socket_address;
    // unsigned char src_mac[6];
    // unsigned char dest_mac[6];
    
    // for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next)
    // {
        
        // if( strcmp(hwa->if_name,"lo") != 0 && strcmp(hwa->if_name,"eth0") != 0 && hwa->if_index == routing_table[client_buffer.destination_index-1].outgoing_index)
        // {
            
            // ptr = hwa->if_haddr;
            // i = IF_HADDR;
            // j=0;
            // /*Loading source Mac Address*/
            // do{
                // src_mac[j] = *ptr++ & 0xff;
            // } while (--i > 0 && j++ < 5);
            
            
            
            // /*buffer for ethernet frame*/
            // void* buffer = (void*)malloc(ETH_FRAME_LEN);
            
            // /*pointer to ethenet header*/
            //unsigned char* etherhead = buffer;
            // ptrethframehdr_send = buffer;
            
            // /*userdata in ethernet frame*/
            // ptrethpayload_send = (buffer + sizeof(struct Eth_Frame_Header));
            
            //Loading values to ethernet header
            // ptrethframehdr_send->packettyped=htons(PROTOCOL);
            // ptrethframehdr_send->datatype=DATA;
            // ptrethframehdr_send->broadcastID=0;
			// ptrethframehdr_send->sourceport = SOURCEPORT;
			// ptrethframehdr_send->destport = client_buffer.serverport;
            // ptrethframehdr_send->RREPsentflag=0;
            // ptrethframehdr_send->hopCount=1;
            // ptrethframehdr_send->forcediscovery=0;
            // ptrethframehdr_send->payloadlen=100;
            // strcpy(ptrethframehdr_send->DestCanonicalIP,client_buffer.destCanonicalIP);
            // strcpy(ptrethframehdr_send->SrcCanonicalIP,client_buffer.sourceCanonicalIP);
            
            // int send_result = 0;
            
            
            // /*other host MAC address*/
            
            // dest_mac[0]  = routing_table[client_buffer.destination_index-1].next_hop[0] & 0xff;
            // dest_mac[1]  = routing_table[client_buffer.destination_index-1].next_hop[1] & 0xff;
            // dest_mac[2]  = routing_table[client_buffer.destination_index-1].next_hop[2] & 0xff;
            // dest_mac[3]  = routing_table[client_buffer.destination_index-1].next_hop[3] & 0xff;
            // dest_mac[4]  = routing_table[client_buffer.destination_index-1].next_hop[4] & 0xff;
            // dest_mac[5]  = routing_table[client_buffer.destination_index-1].next_hop[5] & 0xff;
            // /*MAC - end*/
            
            // /*RAW communication*/
            // socket_address.sll_family   = PF_PACKET;
            // /*we don't use a protocoll above ethernet layer
             // ->just use anything here*/
            // socket_address.sll_protocol = htons(PROTOCOL);
            
            // /*index of the network device
             // see full code later how to retrieve it*/
            // socket_address.sll_ifindex  = hwa->if_index;
            
            // /*ARP hardware identifier is ethernet*/
            // socket_address.sll_hatype   = ARPHRD_ETHER;
            
            // /*target is another host*/
            // socket_address.sll_pkttype  = PACKET_OTHERHOST;
            
            // /*address length*/
            // socket_address.sll_halen    = ETH_ALEN;
            
            
            
            // /*Loading - Next Hop Address*/
            // socket_address.sll_addr[0]  = routing_table[client_buffer.destination_index-1].next_hop[0] & 0xff;
            // socket_address.sll_addr[1]  = routing_table[client_buffer.destination_index-1].next_hop[1] & 0xff;
            // socket_address.sll_addr[2]  = routing_table[client_buffer.destination_index-1].next_hop[2] & 0xff;
            // socket_address.sll_addr[3]  = routing_table[client_buffer.destination_index-1].next_hop[3] & 0xff;
            // socket_address.sll_addr[4]  = routing_table[client_buffer.destination_index-1].next_hop[4] & 0xff;
            // socket_address.sll_addr[5]  = routing_table[client_buffer.destination_index-1].next_hop[5] & 0xff;
            // /*MAC - end*/
            // socket_address.sll_addr[6]  = 0x00;/*not used*/
            // socket_address.sll_addr[7]  = 0x00;/*not used*/
            
            
            // /*set the frame header*/
            // memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
            // memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
            
            // strcpy(ptrethpayload_send->payload,client_buffer.message);
            // printf("$$$$$\n\n Sending Initial Buffer\n\n");
            // printf("\n Sending intial Data : %s \n",ptrethpayload_send->payload);
            // /*send the packet*/
            // send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 // (struct sockaddr*)&socket_address, sizeof(socket_address));
            // if (send_result == -1) {
                // printf("\n Sending error : %d \n",errno);
                // exit(1);
                
            // }
            
        // }
    // }
	
// }
int check_stale(float stalenessT_out)
{
    
    struct timeval tv1;
    time_t time_now;
    
    gettimeofday(&tv1, NULL);
    time_now=tv1.tv_sec*(10^6)+tv1.tv_usec;
    
    
    
    printf("\n Checking for staleness\n");
    
    if((time_now-stalenessT_out*(10^6))>=routing_table[client_buffer.destination_index-1].time_stamp) // Route is stale
    {
        return 1;
    }

    else
    {
        return 0; //Route not stale
    }

}

int main (int argc, char **argv)
{
    
    struct sockaddr_ll rcv_pkt_addr;
    struct hostent *he,*he1;
    char unixbuffer[MAXLINE];
    int maxfdp,nready,unix_data;
    char clientodrvm[5],serverodrvm[5];
    fd_set rset;
    char vm_string[2],vm_string1[2];
    char **ip;
    struct in_addr ipv4addr;
    struct sockaddr_ll temp;
    char mesg[MAXLINE];
    void* buffer;
    char temp_string1[4],temp_string2[4];
    char serverpayload[MAXLINE];
    char odrvm1[5];
    
    
    client_buffer.broadcast=0;
    
    memset(clientodrvm, 0, sizeof clientodrvm);
    
    if(argc < 2)
    {
        printf("error");
    }
    
    stalenessT_out = atoi(argv[1]);
    printf("\n Staleness timeout : %f sec \n", stalenessT_out);
    
    //get hardware address of node's interfaces
    prhwaddrs();
    
    //creating pf_packet socket - packet interface on device level.
    int packet_socket = socket(PF_PACKET, SOCK_RAW, htons(PROTOCOL));
    if (packet_socket == -1)
    {
        printf("Error in creating pf_packet socket \n");
    }
    
    //creating unix domain socket
    struct sockaddr_un odraddr,cliaddr;
    
    int unixdomain_socket= socket(AF_LOCAL, SOCK_DGRAM, 0);
    if(unixdomain_socket < 0){
        printf("\n Unix Domain Socket creation error\n");
        
    }
    
    unlink(ODR_PATH);
    bzero(&odraddr, sizeof(odraddr));
    odraddr.sun_family = AF_LOCAL;
    strcpy(odraddr.sun_path, ODR_PATH);
    
    if(bind(unixdomain_socket, (struct sockaddr *)&odraddr, sizeof(odraddr))<0){
        printf("Unix Domain Socket bind error \n");
    }

    //check on which socket request is coming through select
    while(1)
    {
        FD_ZERO(&rset);
        FD_SET(packet_socket, &rset);
        FD_SET(unixdomain_socket, &rset);
        maxfdp = max(packet_socket,unixdomain_socket) +1;
        nready = select(maxfdp, &rset, NULL, NULL, NULL);
        if ((nready = select(maxfdp, &rset, NULL, NULL, NULL)) < 0)
        {
            printf(" Select error: %d\n", errno);
            continue;
        }
        
        //if request is received on unix domain socket
        if (FD_ISSET(unixdomain_socket, &rset))
        {
            memset(unixbuffer, 0, sizeof(MAXLINE));
            socklen_t sock_len = sizeof(struct sockaddr);;
            unix_data = Recvfrom(unixdomain_socket, unixbuffer , MAXLINE, 0, (struct sockaddr *)&cliaddr, &sock_len);
            if(unix_data < 0)
            {
                printf("error in receiving from unix domain socket\n");
            }
			printf("sun path is %s \n" ,cliaddr.sun_path);
			if(strcmp(cliaddr.sun_path,SERVERPATH)==0){
				printf("received data from server %s \n" , unixbuffer);
				gethostname(serverodrvm, sizeof serverodrvm);
				printf("\n serverodrvm: %s\n", serverodrvm);
				exit(1);
				//senddata_client(unixbuffer,packet_socket,serverodrvm);
				
			} 
			else{
			
				printf("received data from client %s \n" , unixbuffer);
				client_buffer.broadcast++;
				memset(clientodrvm,0,sizeof(clientodrvm));
				gethostname(clientodrvm, sizeof clientodrvm);
				printf("\n clientodrvm: %s\n", clientodrvm);
				    
				client_buffer.source_index=get_index(clientodrvm);
				printf("\n Client index is %d \n",client_buffer.source_index);
				//get client odr canonical IP address
				
				
				he = gethostbyname(clientodrvm);
				if (he == NULL) { // do some error checking
					herror("gethostbyname");
					exit(1);
				}
				
				ip=he->h_addr_list;
				printf("\n source Canonical IP : %s \n",inet_ntop(he->h_addrtype,*ip,client_buffer.sourceCanonicalIP,sizeof(client_buffer.sourceCanonicalIP)));
				
				
				split_buffer(unixbuffer); // Function to split the buffer
                
                
				
				if(client_buffer.sourceCanonicalIP==client_buffer.destCanonicalIP)
				{
					printf(" \n Client is the destination. The buffer is %s",client_buffer.message);
					continue;
				}
                
                else if(client_buffer.flag==1) //Forced Rediscovery mode
                {
                    printf("\n Forced rediscovery mode \n Flooding RREQ on all interfaces to perform route rediscovery \n");
                    initialRREQ(packet_socket,-10,client_buffer.flag);
                    
                }
				
				else if(strcmp(routing_table[client_buffer.destination_index-1].destination_IP,client_buffer.destCanonicalIP)==0)
				{
                    if(check_stale(stalenessT_out)==1) // Checking for staleness.
                    {
                        printf("\n Route found but stale. Flooding RREQ on all interfaces to perform route discovery \n");
                        initialRREQ(packet_socket,-10,client_buffer.flag);
                    }
                    
                    else if(check_stale(stalenessT_out)==0)
                    {
					printf("\n Route Found and not stale. Route Discovery not required. Sending Payload\n");
					send_payload(packet_socket);
					continue;
                    }
					
				}
				
				else if (strcmp(routing_table[client_buffer.destination_index-1].destination_IP,client_buffer.destCanonicalIP)!=0)
				{
					printf("\n No Route found to destination. Flooding RREQ on all interfaces to perform route discovery \n");
					initialRREQ(packet_socket,-10,client_buffer.flag);
				}
            
			
			}
				
		

        }
        
        if (FD_ISSET(packet_socket, &rset))
        {
            
            buffer = (void*)malloc(ETH_FRAME_LEN); /*Buffer for ethernet frame*/
            memset(&rcv_pkt_addr, 0, sizeof(rcv_pkt_addr));
            memset(buffer, 0, sizeof(buffer));
            
            socklen_t rcvlen = sizeof(rcv_pkt_addr);
            
            
            ptrethframehdr_rcv = buffer;
            
            /*userdata in ethernet frame*/
            ptrethpayload_rcv = (buffer + sizeof(struct Eth_Frame_Header));
            int length = 0; /*length of the received frame*/
            length = recvfrom(packet_socket, buffer, ETH_FRAME_LEN, 0,(struct sockaddr*)&rcv_pkt_addr,&rcvlen);
            
            if (length == -1)
            {
                printf("receive error %d",errno);
                exit(1);
                
            }
            
            
            memset(odrvm1,0,sizeof(odrvm1));
            gethostname(odrvm1, sizeof odrvm1);
            printf("odrvm: %s\n", odrvm1);
            
            
            //Loading ODR index
            client_buffer.odr_index=get_index(odrvm1);
            printf("\n ODR index is %d \n",client_buffer.odr_index);
            
            strcpy(mesg,ptrethpayload_rcv->payload);
            
            printf("\n Printing MESSAGE on arrival of packet at ODR %s$$$$$$$$$$ \n\n",mesg);
            
            //Loading Client buffer
            strcpy(client_buffer.destCanonicalIP,ptrethframehdr_rcv->DestCanonicalIP);
            strcpy(client_buffer.sourceCanonicalIP,ptrethframehdr_rcv->SrcCanonicalIP);
            
            inet_pton(AF_INET,client_buffer.destCanonicalIP, &ipv4addr);
            he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
            
            
            //Loading Destination Index for each ODR
            client_buffer.destination_index=get_index(he->h_name);
            
            printf("\n Server index loaded on arrival of ODR is %d \n",client_buffer.destination_index);
            
            
            
            
            inet_pton(AF_INET,client_buffer.sourceCanonicalIP, &ipv4addr);
            he1 = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
            
            
            //Loading Source Index for each ODR
            client_buffer.source_index=get_index(he1->h_name);
            
            printf("\n Client index loaded on arrival of ODR is %d \n",client_buffer.source_index);
            
            
            
            
            if(ptrethframehdr_rcv->datatype == RREQ)
            {
                check_rreq(rcv_pkt_addr,buffer,packet_socket);
            }
            
            
            else if(ptrethframehdr_rcv->datatype == RREP)
            {
                
                if(routing_table[client_buffer.destination_index-1].number_hops==0 || routing_table[client_buffer.destination_index-1].number_hops>ptrethframehdr_rcv->hopCount) // Received packet has lower hopcount
                {
                    
                    
                    if(client_buffer.odr_index==client_buffer.source_index)
                    {
                        //print_ethernetframe(length);
                        printf("\n $$$$$$$$ RREP received at the client ODR $$$$$$$$$$$$$ \n");
                        printf("\n Now Sending payload to server through optimized path \n");
                        
                        add_routing_table(rcv_pkt_addr,FORWARD);//Updating forward routing table at the client
                        send_payload(packet_socket);
                        continue;
                    }
                    
                    else if(client_buffer.odr_index!=client_buffer.source_index)
                    {
                        //print_ethernetframe(length);
                        
                        printf("\n RREP arrived at odr vm%d for client vm%d and destination vm%d\n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
                        add_routing_table(rcv_pkt_addr,FORWARD);//Updating forward routing table at the client
                        relay_rrep(rcv_pkt_addr,packet_socket);
                        
                    }
                }
                
                else if(routing_table[client_buffer.destination_index-1].number_hops<=ptrethframehdr_rcv->hopCount)
                {
                    printf("\n Less efficient path duplicate RREP received. Discarding the duplicate RREP at ODR vm%d \n",client_buffer.odr_index);
                    continue;
                }
            }
            
            
            else if(ptrethframehdr_rcv->datatype == DATA)
            {
                printf("\n New Payload arrived at vm%d from client vm%d for destination vm%d \n",client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
              
                if(client_buffer.odr_index==client_buffer.destination_index)
                {
                    printf("\n Payload received at the server \n");
                    printf("\n Received message %s \n",mesg);
                    snprintf(serverpayload, sizeof(serverpayload), "%d %s %d %s %d %d %d %s", 
					ptrethframehdr_rcv->datatype,
					ptrethframehdr_rcv->SrcCanonicalIP,
					ptrethframehdr_rcv->sourceport,
					ptrethframehdr_rcv->DestCanonicalIP,
					ptrethframehdr_rcv->destport,
					ptrethframehdr_rcv->hopCount,
					ptrethframehdr_rcv->payloadlen,
					ptrethpayload_rcv->payload);
					senddata_server(unixdomain_socket,serverpayload);
                }
                
                else
                {
                    printf("\n Payload $$$$$%s$$$$$$ arrived at vm%d from client vm%d for destination vm%d \n",mesg,client_buffer.odr_index,client_buffer.source_index,client_buffer.destination_index);
                     add_routing_table(rcv_pkt_addr,DATA);
                    relay_payload(rcv_pkt_addr,packet_socket);
                    
                }
            }
            
            
            
            
        }
        
    }
}








