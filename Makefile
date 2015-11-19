CC = gcc

FLAGS = -g 

CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e/lib

LIBUNP_NAME = /home/courses/cse533/Stevens/unpv13e/libunp.a

LIBS = ${LIBUNP_NAME} -lpthread

all: client_astayal odr_astayal
 
client_astayal: client.o api.o
	${CC} ${FLAGS} -o client_astayal client.o api.o ${LIBS}
client.o: client.c
	${CC} ${CFLAGS} -c client.c
api.o: api.c
	${CC} ${CFLAGS} -c api.c
	
odr_astayal: odr.o get_hw_addrs.o
	${CC} ${FLAGS} -o odr_astayal odr.o get_hw_addrs.o ${LIBS}
odr.o: odr.c
	${CC} ${CFLAGS} -c odr.c
get_hw_addrs.o: get_hw_addrs.c
	${CC} ${CFLAGS} -c get_hw_addrs.c
clean:
	rm odr_astayal odr.o client_astayal client.o get_hw_addrs get_hw_addrs.o api api.o

