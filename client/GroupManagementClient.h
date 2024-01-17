#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // for close <not make warning if close a socket>

#define MSG_TRUE "1"
#define MSG_FALSE "0"
#define MSG_DUP "DUPPLICATE"
#define MSG_BLOCKED "BLOCKED"
#define MSG_ERROR "ERROR"

#include "FileManagementClient.h"

int homepage(int client_sock, char *sessionID);
void createGroup(int client_sock, char *owner);
void getIntoGroup(int client_sock, char *owner, int *status, char **sessionID);
void searchGroup(int client_sock);
void groupManager(int client_sock);
void getListGroup(int client_sock);
void getGroupMember(int client_sock);
void invitationProcessing(int client_sock, char *sessionID);
void leaveGroup(int client_sock);