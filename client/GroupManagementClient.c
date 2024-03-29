#include "GroupManagementClient.h"

void createGroup(int client_sock, char *owner)
{ // done
	char groupName[255];
	char buffer[BUFF_SIZE];
	int bytes_sent, bytes_received;

	// if already logged in, cannot registerAccount
	/*if(*status == 1) {
		fprintf(stderr, "You are already logged in. Cannot register account.\n");
		bytes_sent = send(client_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
		if (bytes_sent <= 0) {
			fprintf(stderr, "Failed to connect to server. Try again.\n");
			return;
		}
		return;
	}*/

	// send username to server
	while (1)
	{
		printf("Please enter the group name: ");
		fgets(groupName, 255, stdin);
		groupName[strlen(groupName) - 1] = '\0';

		// enter nothing, cancel registerAccount
		if (groupName[0] == '\0')
		{
			bytes_sent = send(client_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to connect to server. Try again.\n");
				return;
			}
			return;
		}

		bytes_sent = send(client_sock, groupName, strlen(groupName), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to connect to server. Try again.\n");
			return;
		}

		bzero(buffer, BUFF_SIZE);
		bytes_received = recv(client_sock, buffer, BUFF_SIZE, 0);
		if (bytes_received <= 0)
		{
			fprintf(stderr, "Failed to verify username. Try again.\n");
			return;
		}
		else
			buffer[bytes_received] = '\0';

		if (strcmp(buffer, MSG_DUP) == 0)
		{
			fprintf(stderr, "Group existed.\n");
			continue;
		}
		else if (strcmp(buffer, MSG_ERROR) == 0)
		{
			fprintf(stderr, "Error occurred. Try again.\n");
			return;
		}
		else
			break;
	}

	bytes_sent = send(client_sock, owner, strlen(owner), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to connect to server. Try again.\n");
		return;
	}

	bzero(buffer, BUFF_SIZE);
	bytes_received = recv(client_sock, buffer, BUFF_SIZE, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to verify group permission. Try again.\n");
		return;
	}
	else
		buffer[bytes_received] = '\0';

	if (strcmp(buffer, MSG_ERROR) == 0)
		fprintf(stderr, "Error occurred. Try again.\n");
	else
		printf("Successful create group.\n");

	return;
}

void getIntoGroup(int client_sock, char *owner, int *status, char **sessionID)
{ // done
	char groupName[255];
	char buffer[BUFF_SIZE];
	int bytes_sent, bytes_received;

	if (*status == 1)
	{
		fprintf(stderr, "You are already get into %s\n", *sessionID);
		bytes_sent = send(client_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to connect to server. Try again.\n");
			return;
		}
		return;
	}
	// send username to server
	printf("Please enter the group name: ");
	fgets(groupName, 255, stdin);
	groupName[strlen(groupName) - 1] = '\0';
	// enter nothing, cancel signInAccount
	if (groupName[0] == '\0')
	{
		bytes_sent = send(client_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to connect to server. Try again.\n");
			return;
		}
		return;
	}

	bytes_sent = send(client_sock, groupName, strlen(groupName), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to connect to server. Try again.\n");
		return;
	}

	bzero(buffer, BUFF_SIZE);
	bytes_received = recv(client_sock, buffer, BUFF_SIZE, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to verify group name. Try again.\n");
		return;
	}
	else
		buffer[bytes_received] = '\0';

	if (strcmp(buffer, MSG_FALSE) == 0)
	{ // account not found
		fprintf(stderr, "Group not found. Try again.\n");
		return;
	}
	else if (strcmp(buffer, MSG_ERROR) == 0)
	{ // Other error occurred
		fprintf(stderr, "Error occurred. Try again.\n");
		return;
	}

	// send owner name to server to verify owner of folder
	bytes_sent = send(client_sock, owner, strlen(owner), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to connect to server. Try again.\n");
		return;
	}

	bzero(buffer, BUFF_SIZE);
	bytes_received = recv(client_sock, buffer, BUFF_SIZE, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to verify account. Try again.\n");
		return;
	}
	else
		buffer[bytes_received] = '\0';

	printf("You logged into group: %s\n", buffer);
	*status = 1;
	// groupID will be kept at sessionID(groupID)
	*sessionID = strdup(buffer); // save group name are being logged in

	return;
}

void searchGroup(int client_sock)
{ // done
	char groupName[255];
	char buffer[BUFF_SIZE];
	int bytes_sent, bytes_received;

	printf("Please enter the group name: ");
	fgets(groupName, 255, stdin);
	groupName[strlen(groupName) - 1] = '\0';
	// enter nothing, cancel searchAccount
	if (groupName[0] == '\0')
	{
		bytes_sent = send(client_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to connect to server. Try again.\n");
			return;
		}
		return;
	}

	bytes_sent = send(client_sock, groupName, strlen(groupName), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to connect to server. Try again.\n");
		return;
	}

	bytes_received = recv(client_sock, buffer, BUFF_SIZE, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to verify username. Try again.\n");
		return;
	}
	else
		buffer[bytes_received] = '\0';

	if (strcmp(buffer, MSG_TRUE) == 0) // return true mean user found and active
		printf("Group %s is existed.\n", groupName);
	else
		printf("Cannot find group %s.\n", groupName);

	return;
}

void groupManager(int client_sock, char *username)
{
	int bytes_sent;
	char buff[BUFF_SIZE];

	// int status;
	int choice;
	int cache;

	// Step 4: Communicate with server
	// send message
	do
	{
		printf("Group manager\n");
		printf("---------------------------------------------\n");
		printf("	1. File Manager.\n");
		printf("	2. Member.\n");
		printf("	3. Approve.\n");
		printf("	4. Delete member.\n");
		printf("	5. Leave.\n");
		printf("Your choice (1-5, other to quit):\n");
		scanf("%d", &choice);
		while ((cache = getchar()) != '\n' && cache != EOF)
			;
		// send choice to server
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%d", choice);
		bytes_sent = send(client_sock, buff, strlen(buff), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
			return;
		}

		switch (choice)
		{
		case 1:
			fileManager(client_sock);
			break;
		case 2:
			getGroupMember(client_sock);
			break;
		case 3:
			printf("Dang phat trien. Moi tham gia");
			break;
		case 4:
			deleteMember(client_sock, username);
			break;
		case 5:
			leaveGroup(client_sock, username);
			return;
			break;
		default:
			break;
		}

	} while (choice > 0 && choice < 6);

	return;
}

void getListGroup(int client_sock)
{
	int bytes_sent, bytes_received;
	char buff[BUFF_SIZE];
	const char *data_to_send = "list_group";
	bytes_sent = send(client_sock, data_to_send, strlen(data_to_send), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to connect to server. Try again.\n");
		return;
	}

	bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed. Try again.\n");
		return;
	}
	else
		buff[bytes_received] = '\0';

	printf("List Group: \n");
	printf("%s", buff);

	return;
}

void getGroupMember(int client_sock)
{
	char groupName[255];
	char buffer[BUFF_SIZE];
	int bytes_sent, bytes_received;

	bzero(buffer, BUFF_SIZE);
	bytes_received = recv(client_sock, buffer, BUFF_SIZE, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to verify username. Try again.\n");
		return;
	}
	else
	{
		buffer[bytes_received] = '\0';
	}
	if (strcmp(buffer, MSG_FALSE) == 0)
	{
		fprintf(stderr, "Group not found.\n");
		return;
	}
	else if (strcmp(buffer, MSG_ERROR) == 0)
	{
		fprintf(stderr, "Error occurred. Try again.\n");
		return;
	}
	else
	{
		printf("%s", buffer);
	}
}

void leaveGroup(int client_sock, char *owner)
{
	char buffer[BUFF_SIZE];
	int bytes_sent, bytes_received;
	bytes_sent = send(client_sock, owner, strlen(owner), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to connect to server. Try again.\n");
		return;
	}
}

void deleteMember(int client_sock, char *owner)
{
	char buffer[BUFF_SIZE];
	int bytes_sent, bytes_received, permission;

	// Gửi thông tin về owner cho server
	bytes_sent = send(client_sock, owner, strlen(owner), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to send owner information to server. Try again.\n");
		return;
	}

	// Nhận permission từ server
	bzero(buffer, BUFF_SIZE);
	bytes_received = recv(client_sock, buffer, BUFF_SIZE - 1, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive permission from server. Try again.\n");
		return;
	}
	else
	{
		buffer[bytes_received] = '\0';
		permission = atoi(buffer);
	}

	// Kiểm tra permission từ server
	if (permission == 1)
	{
		// Yêu cầu người dùng nhập username cần xóa
		printf("Enter the username to delete: ");
		fflush(stdout);

		bzero(buffer, BUFF_SIZE);
		fgets(buffer, BUFF_SIZE - 1, stdin);

		// Gửi username cần xóa về server
		bytes_sent = send(client_sock, buffer, strlen(buffer), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to send username to delete to server. Try again.\n");
			return;
		}
	}
	else
	{
		// In thông báo nếu không có quyền
		printf("You don't have permission to delete members.\n");
	}
}
