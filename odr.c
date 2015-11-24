#include "hw_addrs.h"



//constants
#define PROTOCOL 562357
#define ODR_PATH "/tmp/astayal"
#define ETH_FRAME_LEN 1500
#define RREQ 0
#define RREP 1
#define DATA 2
#define FORWARD 0
#define REVERSE 1

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
    int destination_index;
    int odr_index;
}client_buffer;




struct Eth_Frame_Header ethframehdr,*ptrethframehdr_send,*ptrethframehdr_rcv;
struct Eth_Frame_Payload ethpayload,*ptrethpayload_send,*ptrethpayload_rcv;




void print_routingtable(int index)
{
    
    int i;
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
        routing_table[index].time_stamp=tv.tv_sec;
        
        //strftime(timebuffer,30,"%m-%d-%Y  %T.",localtime(&routing_table[index].time_stamp));
        
        
        printf("\n Adding Data to Routing Table \n");
        print_routingtable(index);
        printf("\n Time of adding entry to table %ld\n",tv.tv_usec);
        
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
            
            
            printf("\n Adding Data to Routing Table \n");
            print_routingtable(index);
            printf("\n Time of adding entry to table %ld\n",tv.tv_usec);
            
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
            printf("\ninterface index = %d\n\n", hwa->if_index);
        }
    }
    free_hwa_info(hwahead);
}

int initialRREQ(int packet_socket,int skip_index)
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
            ptrethframehdr_send->broadcastID=1;
            ptrethframehdr_send->hopCount=1;
            ptrethframehdr_send->forcediscovery=client_buffer.flag;
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

int floodRREQ(void* buffer, int packet_socket,int skip_index)
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
    char vm_string[MAXLINE];
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
    
    //printf("\n Message received is %s \n",msgrcvd[1]);
    
    inet_pton(AF_INET,msgrcvd[1], &ipv4addr);
    he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
    
    printf("Server name: %s\n",he->h_name);
    
    //strcpy(serverodrvm,he->h_name);
    
    strncpy(vm_string, (he->h_name) + 2, strlen(he->h_name)- 2);
    //printf("\n Server VM string is %s \n",vm_string);
    client_buffer.destination_index=atoi(vm_string);
    
    printf("\n Server index is %d \n",client_buffer.destination_index);
    
    
}

//Checking for a path
int checkRoute(char destIP[],struct sockaddr_ll rcv_pkt_addr)
{
    int i;
    char odrvm[MAXLINE];
    char vm_string[MAXLINE];
    
    
    gethostname(odrvm, sizeof odrvm);
    printf("odrvm: %s\n", odrvm);
    strncpy(vm_string, odrvm + 2, strlen(odrvm)- 2);
    //printf("\n Server VM index is %s \n",vm_string);
    client_buffer.odr_index=atoi(vm_string);
    printf("\n ODR index is %d \n",client_buffer.odr_index);
    
    
    //Check if the ODR itself is the server
    
    if(client_buffer.odr_index==client_buffer.destination_index)
    {
        add_routing_table(rcv_pkt_addr,REVERSE);
        //Updating Destination Routing Table.
        strcpy(routing_table[client_buffer.odr_index-1].destination_IP,destIP);
        printf("\n I am the destination. Sending RREP \n");
        return 1;
    }
    
    for(i=0;i<N_INTERFACES;i++)
    {
        if(strcmp(destIP,routing_table[i].destination_IP) == 0)
        {
            table_index=i;  //Setting table index
            printf("\n route to destination found\n");
            return 1;
        }
    }
    
    return 0;
    
    
}

int newRREQ()
{
    int index=client_buffer.source_index-1;
    if(ptrethframehdr_rcv->broadcastID > routing_table[index].broadcastID[index])
    {
        printf("\n\n New RREQ received -  Updating routing table \n\n");
        return 1;
        
    }
    
    else if(ptrethframehdr_rcv->broadcastID <= routing_table[index].broadcastID[index])
    {
        //Check for hop count condition for effiecent path
        if(ptrethframehdr_rcv->hopCount<=routing_table[index].number_hops)
        {
            printf("\n Path found with better hop count - Updating routing table \n");
            return 1;
            
        }
        
        else
            return 0;
        
    }
    
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
            
            printf("sending relay RREP buffer : %s\n",ptrethpayload_send->payload);
            /*send the packet*/
            send_result = sendto(packet_socket, buffer, ETH_FRAME_LEN, 0,
                                 (struct sockaddr*)&socket_address, sizeof(socket_address));
            if (send_result == -1) {
                printf("Sending error : %d",errno);
                exit(1);
                
            }
            
            
            
            
        }
    }
    
    add_routing_table(rcv_pkt_addr,FORWARD);
    
    
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




int rreq( struct sockaddr_ll rcv_pkt_addr,void* buffer,int packet_socket)
{
    
    int skip_index=rcv_pkt_addr.sll_ifindex;
    
    
    if(checkRoute(ptrethframehdr_rcv->DestCanonicalIP,rcv_pkt_addr)==1)
    {
        printf("\n Route to Destination found. Sending rrep \n");
        sendrrep(packet_socket);
        return 0;
    }
    
    else if(checkRoute(ptrethframehdr_rcv->DestCanonicalIP,rcv_pkt_addr)==0)
    {
        printf("\n No Route found to destination. Checking for efficient path \n");
        if(newRREQ()==1) // Better RREQ received
        {
            add_routing_table(rcv_pkt_addr,REVERSE);
            
            floodRREQ(buffer,packet_socket,skip_index);
        }
        
        return 0;
    }
    
    return 0;
}

void print_ethernetframe(int length)
{
   // printf("length of received buffer : %d\n",length);
    //printf("\n received buffer : %s\n",ptrethpayload_rcv->payload);
    printf("\n received  packet type: %d\n",ptrethframehdr_rcv->packettyped);
    printf("\n received data type: %d\n",ptrethframehdr_rcv->datatype);
    printf("\n received broadcast id : %d\n",ptrethframehdr_rcv->broadcastID);
    printf("\n received hop count : %d\n",ptrethframehdr_rcv->hopCount);
    printf("\n received flag : %d\n",ptrethframehdr_rcv->forcediscovery);
    printf("\n received destination canonical IP : %s\n",ptrethframehdr_rcv->DestCanonicalIP);
    printf("\n received source canonical IP : %s\n",ptrethframehdr_rcv->SrcCanonicalIP);
    
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
    
    printf("\nUpdating Routing table for free RREP\n");
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
            
            printf("sending intial Data : %s\n",ptrethpayload_send->payload);
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


int main (int argc, char **argv)
{
    
    struct sockaddr_ll rcv_pkt_addr;
    struct hostent *he,*he1;
    char unixbuffer[MAXLINE];
    int maxfdp,nready,unix_data;
    char clientodrvm[MAXLINE];
    fd_set rset;
    char vm_string[4],vm_string1[4];
    char **ip;
    struct in_addr ipv4addr;
    struct sockaddr_ll temp;
    
    
    if(argc < 2)
    {
        printf("error");
    }
    
    stalenessT_out = atoi(argv[1]);
    printf("staleness timeout : %f sec\n", stalenessT_out);
    
    //get hardware address of node's interfaces
    prhwaddrs();
    
    //creating pf_packet socket - packet interface on device level.
    int packet_socket = socket(PF_PACKET, SOCK_RAW, htons(PROTOCOL));
    if (packet_socket == -1)
    {
        printf("error in creating pf_packet socket \n");
    }
    
    //creating unix domain socket
    struct sockaddr_un odraddr,cliaddr;
    
    int unixdomain_socket= socket(AF_LOCAL, SOCK_DGRAM, 0);
    if(unixdomain_socket < 0){
        printf("Unix Domain Socket creation error\n");
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
            printf("error: %d\n", errno);
            continue;
        }
        
        //if request is received on unix domain socket
        if (FD_ISSET(unixdomain_socket, &rset))
        {
            socklen_t sock_len = sizeof(struct sockaddr);;
            unix_data = Recvfrom(unixdomain_socket, unixbuffer , MAXLINE, 0, (struct sockaddr *)&cliaddr, &sock_len);
            if(unix_data < 0)
            {
                printf("error in receiving from unix domain socket\n");
            }
            printf("received data %s \n" , unixbuffer);
            
            gethostname(clientodrvm, sizeof clientodrvm);
            printf("\n clientodrvm: %s\n", clientodrvm);
            strncpy(vm_string, clientodrvm + 2, strlen(clientodrvm)- 2);
            //printf("\n Client VM string is %s \n",vm_string);
            client_buffer.source_index=atoi(vm_string);
            
            printf("\n Client index is %d \n",client_buffer.source_index);
            //get client odr canonical IP address
            he = gethostbyname(clientodrvm);
            if (he == NULL) { // do some error checking
                herror("gethostbyname");
                exit(1);
            }
            
            ip=he->h_addr_list;
            printf("\n source Canonical IP : %s \n",inet_ntop(he->h_addrtype,*ip,client_buffer.sourceCanonicalIP,sizeof(client_buffer.sourceCanonicalIP)));
            
            
            split_buffer(unixbuffer);
            
            
            
            
            
            if(checkRoute(client_buffer.destCanonicalIP,temp)==0)
            {
                printf("\n No Route found to destination. Flooding RREQ on all interfaces \n");
                initialRREQ(packet_socket,-10);
            }
            
        }
        
        if (FD_ISSET(packet_socket, &rset))
        {
            
            memset(&rcv_pkt_addr, 0, sizeof(rcv_pkt_addr));
            socklen_t rcvlen = sizeof(rcv_pkt_addr);
            
            void* buffer = (void*)malloc(ETH_FRAME_LEN); /*Buffer for ethernet frame*/
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
            
            //Loading Client buffer
            strcpy(client_buffer.destCanonicalIP,ptrethframehdr_rcv->DestCanonicalIP);
            strcpy(client_buffer.sourceCanonicalIP,ptrethframehdr_rcv->SrcCanonicalIP);
            
            inet_pton(AF_INET,client_buffer.destCanonicalIP, &ipv4addr);
            he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
            
            printf("Server name: %s\n",he->h_name);
            
            //strcpy(serverodrvm,he->h_name);
            
            strncpy(vm_string, (he->h_name) + 2, strlen(he->h_name)- 2);
            //printf("\n Server VM index is %s \n",vm_string);
            client_buffer.destination_index=atoi(vm_string);
            
            printf("\n Server index is %d \n",client_buffer.destination_index);
            
            
            
            
            inet_pton(AF_INET,client_buffer.sourceCanonicalIP, &ipv4addr);
            he1 = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
            
            printf("Client name: %s\n",he1->h_name);
            
            //strcpy(serverodrvm,he->h_name);
            
            strncpy(vm_string1, (he1->h_name) + 2, strlen(he1->h_name)- 2);
            //printf("\n Client VM index is %s \n",vm_string1);
            client_buffer.source_index=atoi(vm_string1);
            
            printf("\n Client index is %d \n",client_buffer.source_index);
            
            
            print_ethernetframe(length);
            
            if(ptrethframehdr_rcv->datatype == RREQ)
            {
                printf("\n RREQ received \n");
                
                
                rreq(rcv_pkt_addr,buffer,packet_socket);
                
            }
            else if(ptrethframehdr_rcv->datatype == RREP){
               
                if(client_buffer.odr_index==client_buffer.source_index)
                {    printf("\n $$$$$$$$ RREP received at the client ODR $$$$$$$$$$$$$ \n");
                    printf("\n Now Sending payload to server through optimized path \n");
                    add_routing_table(rcv_pkt_addr,FORWARD);//Updating forward routing table at the client
                    send_payload(packet_socket);
                    
                }
                
                else
                {
                    
                relay_rrep(rcv_pkt_addr,packet_socket);
                    
                }
            }
            else if(ptrethframehdr_rcv->datatype == DATA)
            {
                
                
                if(client_buffer.odr_index==client_buffer.destination_index)
                {
                    printf("\n Payload received at the server \n");
                    printf("\n Received message is %s",ptrethpayload_rcv->payload);
                  
                }
                
                else
                {
                    printf("\n Data Received\n");
                    relay_payload(rcv_pkt_addr,packet_socket);
                    
                }
            }
            
            
            
            
        }
        
    }
}








