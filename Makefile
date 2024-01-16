CFLAGS =-Wall
CC = gcc

server: ServerFuncs.o tcp_server.o FileTransferServer.o group_manage_server.o
	${CC} ${CFLAGS} -lpthread tcp_server.o ServerFuncs.o FileTransferServer.o group_manage_server.o -o server

client: ServerAPI.o  TCPClient.o FileManagement.o GroupClient.o
	${CC} ${CFLAGS} TCPClient.o ServerAPI.o FileManagement.o GroupClient.o -o client

tcp_server.o : tcp_server.c
	${CC} -c ${CFLAGS} -lpthread tcp_server.c

TCPClient.o : TCPClient.c
	${CC} -c ${CFLAGS} TCPClient.c

ServerFuncs.o :ServerFuncs.c 
	${CC} -c ${CFLAGS} ServerFuncs.c 

ServerAPI.o: ServerAPI.c 
	${CC} -c ${CFLAGS} ServerAPI.c 

FileTransferServer.o : FileTransferServer.c
	${CC} -c ${CFLAGS} FileTransferServer.c

FileManagement.o: FileManagement.c
	${CC} -c ${CFLAGS} FileManagement.c
	
group_manage_server.o: group_manage_server.c
	${CC} -c ${CFLAGS} group_manage_server.c

GroupClient.o: GroupClient.c
	${CC} -c ${CFLAGS} GroupClient.c

clean: 
	rm -f *.o *~