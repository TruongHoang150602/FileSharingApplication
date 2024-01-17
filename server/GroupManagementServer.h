#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "FileManagementServer.h"

#define MSG_TRUE "1"
#define MSG_FALSE "0"
#define MSG_DUP "DUPPLICATE"
#define MSG_BLOCKED "BLOCKED"
#define MSG_ERROR "ERROR"

typedef struct Member
{
	char *name;
	struct Member *next;
} Member;

typedef struct Approve
{
	char *name;
	struct Approve *next;
} Approve;

typedef struct groupNode
{
	char *groupName;
	char *owner;
	struct Member *members;
	struct Approve *approves;
	struct groupNode *next;
} Group;

void homepage(int sockfd);
void readGroupInfo(Group *root, FILE *fp);
void createGroup(Group *root, int conn_sock, FILE *db);
void getIntoGroup(Group *root, int conn_sock, FILE *db);
void searchGroup(Group *root, int conn_sock);
void freeGroupList(Group *root);
void groupManager(Group *group, int conn_sock, char *path, int permission);
void getListGroup(Group *root, int conn_sock);
void getGroupMember(Group *root, int conn_sock);