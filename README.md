README for CSE 533 Assignment 3 (Network Programming)
-----------------------------------------------------

Submitted by : Karthikeyan Swaminathan (110562357) and Ashana Tayal(110478854)

AIM: To implement a client/server time application in which the meesages are exchanged in a network using an On-Demand shortest-hop Routing (ODR) protocol.

We have successfully implemented the following requirements of the assignment:-

An On-Demand shortest-hop Routing (ODR) protocol for routing messages in networks of fixed but
arbitrary and unknown connectivity, using PF_PACKET sockets. The implementation is based on (a
simplified version of) the AODV algorithm.

A client/server time application in which clients and servers communicate with each other across a
network; the messages exchanged are transmitted using ODR.

An API that enables applications to communicate with the ODR mechanism running locally at their nodes,
using Unix domain datagram sockets.

