README for CSE 533 Assignment 3 (Network Programming)
-----------------------------------------------------

Submitted by : Karthikeyan Swaminathan (110562357) and Ashana Tayal(110478854)

AIM: To implement a client/server time application in which the messages are exchanged in a network using an On-Demand shortest-hop Routing (ODR) protocol.

We have successfully implemented the following requirements of the assignment:-

An On-Demand shortest-hop Routing (ODR) protocol for routing messages in networks of fixed but
arbitrary and unknown connectivity, using PF_PACKET sockets. The implementation is based on (a
simplified version of) the AODV algorithm.

A client/server time application in which clients and servers communicate with each other across a
network; the messages exchanged are transmitted using ODR.

An API that enables applications to communicate with the ODR mechanism running locally at their nodes,
using Unix domain datagram sockets.

4.) ODR 

The ODR process runs on each of the ten vm machines. 
It is evoked with a single command line argument which gives a “staleness” time parameter, in seconds.

USAGE to evoke the ODR process on each VM:

./odr_astayal <staleness parameter>

The ODR process performs the following tasks:

1) It uses get_hw_addrs (available on minix in ~cse533/Asgn3_code) to obtain the index, and associated (unicast) IP and Ethernet addresses for each of the node’s interfaces, except for the eth0 and lo(loopback) interfaces, which are ignored.If an interface has no HW address associated with it,the process prints a blank line instead of its usual ‘HWaddr = xx:xx:xx:xx:xx:xx’.

2) The ODR process creates one PF_PACKET socket of SOCK_RAW type, on the interface where the incoming frame was received, using a sockaddr_ll structure.
The protocol value we use is ‘562357’. We define it as type def value ’PROTOCOL’ in our code.

3)The ODR process also creates a Unix domain datagram socket for communication with application processes at the node, and binds the socket to a ‘well known’ sun_path name for the ODR service.

4)The ODR process uses select to listen on the UNIX and PF_Packet socket.

5)The ODR process builds the routing table whenever the ‘add_routing_table()’ is called. This function constructs the FORWARD or the REVERSE path according to the argument passed to it.


Routing table is an array of structures of routing entries. The number of entries,N_INTERFACES depends on the number of virtual machines. In our case the value of N_INTERFACES is 10.

struct Routing_Table{
	char destination_IP[16];
	char next_hop[8];
	int  outgoing_index; 
	int  number_hops;
	time_t   time_stamp;
    uint16_t broadcastID[N_INTERFACES];
}routing_table[N_INTERFACES];

For each ODR, we create N_INTERFACES number of routing table entries.
In the reverse path construction, the route to the source node is constructed using index of the client. 

In the forward path construction, the route to the destination node is constructed using the index of the destination.

6) The value of client index, odr index and the destination index are computed using the get_index() function and stored in the client_buffer structure.

7) RREQ is sent in response to a time client request, when no route is found. The route is flooded out on all interfaces except for eth0 and lo. This is achieved using the initialRREQ().The ODR at the client node increments the broadcast_id every time it issues a new RREQ for any destination node.

8) When an RREQ node is received at the intermediate node, if its at the destination, its sends an RREP. If its an intermediate node,  and if it has the route, it sends an RREP. Else it propagates the RREQ using the floodRREQ() on all interfaces except the one in which it received the the RREQ on.

9) RREQs with a higher broadcast ID are automatically used to update routing tables. If the RREQs have a lower broadcast ID, we update the routing table only if the new RREQ has a better path(lower hop count). Inefficient path RREQs are discarded.

10) RREPs with the ones higher than what we have in the routing tables are discarded. Once an RREP is sent, the ODR propagates RREQ with RREQalready sent flag set. Nodes which receive this RREQalreadysent flag will only update their routing tables and not send an RREP.

11) The reverse path to the client, is updated when a node receives an acceptable RREQs. RREP is sent along this path and simultaneously the forward path to the destination is updated.

12) Once the RREP reaches the ODR VM on which the client is running on, the payload is sent along the forward path. The routing table is updated for the reverse path simultaneously. This is the FREE RREP implementation.

13) Staleness is checked using check_stale function.

14) Once the payload is received by the on the server ODR, it is forwarded to the server. The server responds to this by sending its time of the day. The odd on the server VM, forwards this time back to the ODR on the client VM using the reverse path. 

15) Once the ODR on the client VM receives the time of the day, it forwards this to the client which prints it out on its console.

Parts that are not implemented:
Staleness
Force Discovery
ODR port API





