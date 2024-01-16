CFLAGS =-Wall
CC = gcc

client: ServerAPI.o  TCPClient.o FileManagementClient.o GroupManagementClient.o
	${CC} ${CFLAGS} TCPClient.o ServerAPI.o FileManagementClient.o GroupManagementClient.o -o client

TCPClient.o : TCPClient.c
	${CC} -c ${CFLAGS} TCPClient.c

ServerAPI.o: ServerAPI.c 
	${CC} -c ${CFLAGS} ServerAPI.c 

FileManagementClient.o: FileManagementClient.c
	${CC} -c ${CFLAGS} FileManagementClient.c
	
GroupManagementClient.o: GroupManagementClient.c
	${CC} -c ${CFLAGS} GroupManagementClient.c

clean: 
	rm -f *.o *~