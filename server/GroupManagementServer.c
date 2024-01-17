#include "GroupManagementServer.h"

// Group function
void readGroupInfo(Group *root, FILE *fp)
{
	// char groupName[255];
	// char owner[255];
	Group *tmp = root;

	fseek(fp, 0, SEEK_SET);

	char array[256];
	while (fgets(array, 256, fp))
	{
		char *token = strtok(array, " ");
		char *result[256];
		int i = 0;
		while (token != NULL)
		{
			result[i] = token;
			i++;
			token = strtok(NULL, " ");
		}

		Group *newGroup = (Group *)malloc(sizeof(Group));
		newGroup->groupName = strdup(result[0]);
		newGroup->owner = strdup(result[1]);
		newGroup->members = NULL;
		for (int j = 2; j < i; j++)
		{
			Member *newMember = (Member *)malloc(sizeof(Member));
			newMember->name = strdup(result[j]);
			newMember->next = NULL;

			if (newGroup->members == NULL)
			{
				newGroup->members = newMember;
			}
			else
			{
				Member *temp = newGroup->members;
				while (temp->next != NULL)
				{
					temp = temp->next;
				}
				temp->next = newMember;
			}
		}

		newGroup->next = NULL;

		tmp->next = newGroup;
		tmp = tmp->next;
	}

	return;
}

void createGroup(Group *root, int conn_sock, FILE *db)
{ // done
	char groupName[255];
	char owner[255];
	int isDuplicate = 0;
	Group *tmp;

	int bytes_sent, bytes_received;

	// receive group name from client
	while (1)
	{
		isDuplicate = 0; // reset isDuplicate

		bzero(groupName, 255);
		bytes_received = recv(conn_sock, groupName, 256, 0);
		if (bytes_received <= 0)
		{
			fprintf(stderr, "Failed to receive group name from client. Try again.\n");
			return;
		}
		else
			groupName[bytes_received] = '\0';
		// receive null or there's already an user
		if (strcmp(groupName, MSG_FALSE) == 0)
		{
			printf("Cancel create group.\n");
			return;
		}

		if (root->next == NULL)
		{
			fprintf(stderr, "Empty database.\n");
			bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to send signal to client. Try again.\n");
				return;
			}
			return;
		}

		tmp = root->next;

		while (tmp != NULL)
		{
			if (strcmp(tmp->groupName, groupName) == 0)
			{
				isDuplicate = 1;
				break;
			}
			if (tmp->next == NULL)
				break;
			tmp = tmp->next;
		}

		if (isDuplicate)
		{
			printf("Group existed.\n");
			bytes_sent = send(conn_sock, MSG_DUP, strlen(MSG_DUP), 0);
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to send signal to client. Try again.\n");
				return;
			}
		}
		else
		{
			bytes_sent = send(conn_sock, MSG_TRUE, strlen(MSG_TRUE), 0);
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to send signal to client. Try again.\n");
				return;
			}
			break;
		}
	}

	// receive owner from client
	bzero(owner, 64);
	bytes_received = recv(conn_sock, owner, 255, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive owner name from client. Try again.\n");
		return;
	}
	else
		owner[bytes_received] = '\0';

	Group *newGroup = (Group *)malloc(sizeof(Group));

	if (newGroup == NULL)
	{
		perror("\nError: ");
		bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to send signal to client. Try again.\n");
			return;
		}
	}
	else
	{
		newGroup->groupName = strdup(groupName);
		newGroup->owner = strdup(owner);
		newGroup->next = NULL;
		tmp->next = newGroup;

		printf("%s\n", newGroup->groupName);
		printf("%s\n", newGroup->owner);

		fseek(db, 0, SEEK_END);
		int bytes_written = fprintf(db, "%s %s\n", groupName, owner);
		fflush(db);
		mkdir(newGroup->groupName, 0700);
		printf("Total bytes written: %d\n", bytes_written);

		bytes_sent = send(conn_sock, MSG_TRUE, strlen(MSG_TRUE), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to send signal to client. Try again.\n");
			return;
		}
	}

	return;
}

void getIntoGroup(Group *root, int conn_sock, FILE *db)
{
	char groupName[255];
	char owner[255];
	int bytes_sent, bytes_received;
	// int status;
	Group *tmp;

	bzero(groupName, 255);
	bytes_received = recv(conn_sock, groupName, 256, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive group name from client. Try again.\n");
		return;
	}
	else
		groupName[bytes_received] = '\0';

	// receive null or there's already have an user
	if (strcmp(groupName, MSG_FALSE) == 0)
	{
		fprintf(stderr, "Already get into group.\n");
		return;
	}

	if (root->next == NULL)
	{
		fprintf(stderr, "Empty database.\n");
		bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to send signal to client. Try again.\n");
			return;
		}
		return;
	}

	tmp = root->next;

	while (tmp != NULL)
	{
		if (strcmp(tmp->groupName, groupName) == 0)
		{
			printf("Found.\n");
			bytes_sent = send(conn_sock, MSG_TRUE, strlen(MSG_TRUE), 0); // inform that groupName is valid
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to send signal to client. Try again.\n");
				return;
			}

			bzero(owner, 255);
			bytes_received = recv(conn_sock, owner, 255, 0);
			if (bytes_received <= 0)
			{
				fprintf(stderr, "Failed to receive owner from client. Try again.\n");
				return;
			}
			else
				owner[bytes_received] = '\0';

			bytes_sent = send(conn_sock, groupName, strlen(groupName), 0); // send logged group name
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to send signal to client. Try again.\n");
				return;
			}
			// owner and user are matched.
			if (strcmp(tmp->owner, owner) == 0)
			{ // can delete file
				fprintf(stderr, "Client is owner of this group.\n");
				groupManager(tmp, conn_sock, groupName, 1);
				return;
			}
			else
			{
				fprintf(stderr, "Client is not owner of this group.\n");
				groupManager(tmp, conn_sock, groupName, 0);
				return;
			}
		}
		tmp = tmp->next;
	}

	printf("Cannot find group.\n");
	bytes_sent = send(conn_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to send signal to client. Try again.\n");
	}
	return;
}

void searchGroup(Group *root, int conn_sock)
{
	char groupName[255];
	Group *tmp;
	int bytes_sent, bytes_received;

	bytes_received = recv(conn_sock, groupName, 256, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to get group name. Try again.\n");
		return;
	}
	else
		groupName[bytes_received] = '\0';

	// receive null
	if (strcmp(groupName, MSG_FALSE) == 0)
		return;

	if (root->next == NULL)
	{
		fprintf(stderr, "Empty database.\n");
		bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to send signal to client. Try again.\n");
			return;
		}
		return;
	}

	tmp = root->next;

	while (tmp != NULL)
	{
		if (strcmp(tmp->groupName, groupName) == 0)
		{
			printf("Group is existed.\n");
			bytes_sent = send(conn_sock, MSG_TRUE, strlen(MSG_TRUE), 0);
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to send signal to client. Try again.\n");
			}
			return;
		}
		tmp = tmp->next;
	}

	printf("Cannot find group.\n");
	bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to send signal to client. Try again.\n");
		return;
	}
}

void freeGroupList(Group *root)
{
	Group *tmp = root->next;
	Group *guard = tmp;
	if (tmp == NULL)
		return; // empty linked list
	while (tmp != NULL)
	{
		guard = tmp->next;
		free(tmp->groupName);
		free(tmp->owner);
		free(tmp);
		tmp = guard;
	}
}

void groupManager(Group *group, int conn_sock, char *path, int permission)
{
	char recv_data[BUFF_SIZE];
	int bytes_received;

	// int status;
	int choice;

	// start conversation
	do
	{
		printf("Group manager: ");
		// receives message from client
		memset(recv_data, 0, BUFF_SIZE);
		bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0); // blocking
		if (bytes_received <= 0)
		{
			printf("\nConnection closed");
			break;
		}
		printf("%s\n", recv_data);

		choice = atoi(recv_data);
		switch (choice)
		{
		case 1:
			fileManager(conn_sock, path, permission);
			break;
		case 2:
			getGroupMember(group, conn_sock);
			break;
		case 3:
			break;
		case 4:
			printf("Delete member\n");
			deleteMember(group, conn_sock);
			printf("Delete member end\n");
			break;
		case 5:
			printf("Leave");
			leaveGroup(group, conn_sock);
			printf("End leave");
			return;
			break;
		default:
			break;
		}

	} while (choice > 0 && choice < 5);

	return;
}

void getListGroup(Group *root, int conn_sock)
{
	char list_group[1024];
	Group *tmp;
	int bytes_sent, bytes_received;
	char data_received[12];
	bytes_received = recv(conn_sock, data_received, sizeof(data_received) - 1, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed. Try again.\n");
		return;
	}

	if (root->next == NULL)
	{
		fprintf(stderr, "Empty database.\n");
		bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
		if (bytes_sent <= 0)
		{
			fprintf(stderr, "Failed to send signal to client. Try again.\n");
			return;
		}
		return;
	}

	tmp = root->next;

	while (tmp != NULL)
	{
		strcat(list_group, tmp->groupName);
		strcat(list_group, "\n");
		tmp = tmp->next;
	}
	printf("%s", list_group);
	bytes_sent = send(conn_sock, list_group, strlen(list_group), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to send signal to client. Try again.\n");
	}
	return;
}

void getGroupMember(Group *group, int conn_sock)
{
	char groupName[255];
	char list_member[1024] = "Group name: ";
	Group *tmp;
	int bytes_sent;

	if (strcmp(groupName, MSG_FALSE) == 0)
	{
		printf("Cancel");
		return;
	}

	printf("%s %s\n", group->groupName, group->owner);
	strcat(list_member, group->groupName);
	strcat(list_member, "\n");
	strcat(list_member, "Owner: ");
	strcat(list_member, group->owner);
	strcat(list_member, "\n");
	strcat(list_member, "Member: ");
	while (group->members != NULL)
	{
		strcat(list_member, group->members->name);
		strcat(list_member, ", ");
		group->members = group->members->next;
	}

	bytes_sent = send(conn_sock, list_member, strlen(list_member), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Group not found. Try again.\n");
		return;
	}
}

void leaveGroup(Group *group, int conn_sock)
{
	char username[255];
	int bytes_received;
	bzero(username, 255);
	bytes_received = recv(conn_sock, username, 255, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive owner from client. Try again.\n");
		return;
	}
	else
		username[bytes_received] = '\0';

	printf("%s\n", username);

	if (group == NULL || username == NULL)
	{
		// Kiểm tra đối số đầu vào
		printf("Invalid input.\n");
		return;
	}

	Member *currentMember = group->members;
	Member *prevMember = NULL;

	// Duyệt qua danh sách members của nhóm
	while (currentMember != NULL)
	{
		// So sánh tên người dùng
		if (strcmp(currentMember->name, username) == 0)
		{
			if (prevMember == NULL)
			{
				group->members = currentMember->next;
			}
			else
			{
				prevMember->next = currentMember->next;
			}
			free(currentMember->name);
			free(currentMember);
			return;
		}

		prevMember = currentMember;
		currentMember = currentMember->next;
	}
}

void deleteMember(Group *group, int conn_sock)
{
	char username[255];
	int bytes_received, bytes_sent;
	bzero(username, 255);

	bytes_received = recv(conn_sock, username, 255, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive username from client. Try again.\n");
		return;
	}
	else
	{
		username[bytes_received] = '\0';
	}

	printf("%s\n", username);

	int permission = !(strcmp(group->owner, username));
	char permission_str[10];
	snprintf(permission_str, sizeof(permission_str), "%d", permission);

	bytes_sent = send(conn_sock, permission_str, strlen(permission_str), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to send permission status. Try again.\n");
	}
	if (permission == 0)
	{
		return;
	}

	bzero(username, 255);
	bytes_received = recv(conn_sock, username, 255, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive username from client. Try again.\n");
		return;
	}
	else
	{
		username[bytes_received] = '\0';
	}

	Member *currentMember = group->members;
	Member *prevMember = NULL;

	// Duyệt qua danh sách members của nhóm
	while (currentMember != NULL)
	{
		// So sánh tên người dùng
		if (strcmp(currentMember->name, username) == 0)
		{
			if (prevMember == NULL)
			{
				group->members = currentMember->next;
			}
			else
			{
				prevMember->next = currentMember->next;
			}
			free(currentMember->name);
			free(currentMember);
			return;
		}

		prevMember = currentMember;
		currentMember = currentMember->next;
	}
}
