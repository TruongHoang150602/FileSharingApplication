#include "AccountManagementServer.h"
#include <pthread.h>

#define BUFF_SIZE 1024
#define ACCOUNT_FILE_PATH "../account.json"

/*
 * Receive and echo message to client
 * [IN] sockfd: socket descriptor that connects to client
 */
typedef struct arg_struct
{
	int connfd;
	Account *root;
	FILE *fp;
} Args;

void cleanup(int listenfd, FILE *db);
void *handleClient(void *arg);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("No specified port.\n");
		exit(EXIT_FAILURE);
	}

	Account *root = (Account *)calloc(1, sizeof(Account));
	FILE *db = fopen(ACCOUNT_FILE_PATH, "r+");
	if (db == NULL)
	{
		fprintf(stderr, "Cannot open target file %s\n", ACCOUNT_FILE_PATH);
		exit(EXIT_FAILURE);
	}
	readFromFile(root, db);

	int listenfd, sin_port;
	struct sockaddr_in server;
	pthread_t tid;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error creating socket");
		cleanup(listenfd, db);
		exit(EXIT_FAILURE);
	}

	sin_port = atoi(argv[1]);
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(sin_port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Error binding");
		cleanup(listenfd, db);
		exit(EXIT_FAILURE);
	}

	if (listen(listenfd, BACKLOG) == -1)
	{
		perror("Error listening");
		cleanup(listenfd, db);
		exit(EXIT_FAILURE);
	}

	printf("Server is running on port %d\n", sin_port);

	struct sockaddr_in client;
	socklen_t sin_size = sizeof(struct sockaddr_in);

	while (1)
	{
		int *connfd = malloc(sizeof(int));
		if ((*connfd = accept(listenfd, (struct sockaddr *)&client, &sin_size)) == -1)
		{
			perror("Error accepting connection");
			free(connfd);
			continue;
		}

		printf("Connection established from %s\n", inet_ntoa(client.sin_addr));

		Args *arg = (Args *)malloc(sizeof(Args));
		arg->connfd = *connfd;
		arg->root = root;
		arg->fp = db;

		if (pthread_create(&tid, NULL, handleClient, (void *)arg) != 0)
		{
			perror("Error creating thread");
			free(arg);
			close(*connfd);
			free(connfd);
			continue;
		}
	}

	cleanup(listenfd, db);
	return 0;
}

void cleanup(int listenfd, FILE *db)
{
	close(listenfd);
	fclose(db);
	// Add additional cleanup steps as needed
}

void *handleClient(void *arg)
{

	char recv_data[BUFF_SIZE];
	int bytes_received;
	int status;

	int sockfd = ((Args *)arg)->connfd;
	Account *root = ((Args *)arg)->root;
	FILE *db = ((Args *)arg)->fp;

	free(arg);
	pthread_detach(pthread_self());

	Group *rootGroup = (Group *)calloc(1, sizeof(Group));

	FILE *dbGroup = fopen("../group.txt", "r+");
	if (dbGroup == NULL)
	{
		fprintf(stderr, "cannot open group list file\n");
		exit(2);
	}
	readGroupInfo(rootGroup, dbGroup);

	fclose(dbGroup);

	// start conversation
	while (1)
	{
		// receives message from client
		memset(recv_data, 0, BUFF_SIZE);
		bytes_received = recv(sockfd, recv_data, BUFF_SIZE, 0); // blocking
		if (bytes_received <= 0)
		{
			printf("\nConnection closed");
			break;
		}
		printf("%s\n", recv_data);

		switch (atoi(recv_data))
		{
		case 1:
			registerAccount(root, sockfd, db);
			break;
		case 2:
			signInAccount(root, sockfd, db, &status);
			if (status == 1)
				homepage(rootGroup, sockfd);
			break;
		case 3:
			searchAccount(root, sockfd);
			break;
		case 4:
			if (status == 1)
				signOutAccount(root, sockfd, &status);
			break;
		default: // end process
			if (status == 1)
				signOutAccount(root, sockfd, &status);
			// freeAccount(root);
			// return 0;
			//  close connection then exit thread
			close(sockfd);
			pthread_exit(NULL);
		}
	} // end conversation

	close(sockfd);
	pthread_exit(NULL);
}
