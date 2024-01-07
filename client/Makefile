CFLAGS =-Wall
CC = gcc

client: ServerAPI.o  tcp_client.o FileTransferClient.o room_client.o
	${CC} ${CFLAGS} tcp_client.o ServerAPI.o FileTransferClient.o room_client.o -o client

tcp_client.o : tcp_client.c
	${CC} -c ${CFLAGS} tcp_client.c

ServerAPI.o: ServerAPI.c 
	${CC} -c ${CFLAGS} ServerAPI.c 

FileTransferClient.o: FileTransferClient.c
	${CC} -c ${CFLAGS} FileTransferClient.c
	
room_client.o: room_client.c
	${CC} -c ${CFLAGS} room_client.c

clean: 
	rm -f *.o *~