CFLAGS =-Wall
CC = gcc

server: AccountManagementServer.o TCPServer.o FileManagementServer.o GroupManagementServer.o
	${CC} ${CFLAGS} -lpthread TCPServer.o AccountManagementServer.o FileManagementServer.o GroupManagementServer.o -o server -lcjson


TCPServer.o : TCPServer.c
	${CC} -c ${CFLAGS} -lpthread TCPServer.c

TCPClient.o : TCPClient.c
	${CC} -c ${CFLAGS} TCPClient.c

AccountManagementServer.o :AccountManagementServer.c 
	${CC} -c ${CFLAGS} -lcjson AccountManagementServer.c 

ServerAPI.o: ServerAPI.c 
	${CC} -c ${CFLAGS} ServerAPI.c 

FileManagementServer.o : FileManagementServer.c
	${CC} -c ${CFLAGS} FileManagementServer.c

FileManagement.o: FileManagement.c
	${CC} -c ${CFLAGS} FileManagement.c
	
GroupManagementServer.o: GroupManagementServer.c
	${CC} -c ${CFLAGS} GroupManagementServer.c

GroupClient.o: GroupClient.c
	${CC} -c ${CFLAGS} GroupClient.c

clean: 
	rm -f *.o *~