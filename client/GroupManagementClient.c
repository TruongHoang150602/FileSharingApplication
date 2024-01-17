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

void groupManager(int client_sock)
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
			printf("Dang phat trien. Xoa thanh vien");
			break;
		case 5:
			leaveGroup(client_sock);
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

void leaveGroup(int client_sock)
{
	char groupName[255];
	char buffer[BUFF_SIZE];
	int bytes_sent, bytes_received;

	while (1)
	{
		printf("Please enter the group name to leave: ");
		fgets(groupName, 255, stdin);
		groupName[strlen(groupName) - 1] = '\0';

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
			fprintf(stderr, "Failed to verify leave request. Try again.\n");
			return;
		}
		else
		{
			buffer[bytes_received] = '\0';
		}

		if (strcmp(buffer, MSG_FALSE) == 0)
		{
			fprintf(stderr, "Group not found or you are not a member of the group.\n");
			return;
		}
		else if (strcmp(buffer, MSG_ERROR) == 0)
		{
			fprintf(stderr, "Error occurred. Try again.\n");
			return;
		}
		else
		{
			printf("You have left the group: %s\n", groupName);
			break;
		}
	}
}

void invitationProcessing(int client_sock, char *sessionID)
{
	FILE *file = fopen("../account.json", "r");
	if (!file)
	{
		fprintf(stderr, "Không thể mở tệp JSON\n");
		return;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *jsonBuffer = (char *)malloc(fileSize + 1);
	fread(jsonBuffer, 1, fileSize, file);
	fclose(file);

	// Bắt đầu phân tích cú pháp JSON
	cJSON *json = cJSON_Parse(jsonBuffer);
	free(jsonBuffer);

	if (!json)
	{
		fprintf(stderr, "Lỗi phân tích cú pháp JSON\n");
		return;
	}

	// Lặp qua các đối tượng trong mảng JSON
	cJSON *user = NULL;
	char *result;
	cJSON_ArrayForEach(user, json)
	{
		cJSON *username = cJSON_GetObjectItemCaseSensitive(user, "username");
		if (cJSON_IsString(username))
		{
			if (strcmp(username->valuestring, sessionID) == 0)
			{
				cJSON *invitation = cJSON_GetObjectItemCaseSensitive(user, "invitation");
				if (cJSON_IsArray(invitation))
				{
					if (cJSON_GetArraySize(invitation) == 0)
					{
						printf("No invites yet!");
					}
					else
					{
						FILE *file = fopen("../group.txt", "r");
						if (!file)
						{
							fprintf(stderr, "Không thể mở tệp\n");
							return;
						}

						// Tìm kích thước của tệp
						fseek(file, 0, SEEK_END);
						long fileSize = ftell(file);
						fseek(file, 0, SEEK_SET);

						// Đọc nội dung tệp vào bộ đệm
						char *fileContent = (char *)malloc(fileSize + 1);
						fread(fileContent, 1, fileSize, file);
						fileContent[fileSize] = '\0'; // Thêm ký tự null vào cuối bộ đệm
						fclose(file);
						printf("%s", fileContent);
						// Tìm dòng đầu tiên
						char *line = strtok(fileContent, "\n");
						// Mảng để lưu trữ dữ liệu đã sửa
						char **modifiedLines = (char **)malloc(sizeof(char *) * 1000);
						int lineCount = 0;

						cJSON *group = NULL;
						char *copy = (char *)malloc(256);
						char *nameGr = (char *)malloc(256);
						char *modifiedLine = (char *)malloc(256);
						cJSON_ArrayForEach(group, invitation)
						{
							while (line != NULL)
							{
								// Thêm dữ liệu mới vào cuối mỗi dòng
								line[strlen(line) - 1] = '\0';
								copy = (char *)malloc(256);
								strcpy(copy, line);
								printf("%s\n", copy);
								nameGr = strtok(copy, " ");
								printf("nameGr: %s\n", nameGr);
								printf("Group: %s\n", group->valuestring);
								free(copy);
								if (strcmp(nameGr, group->valuestring) == 0)
								{
									printf("IF");
									sprintf(modifiedLine, sessionID, line);
									modifiedLines[lineCount++] = modifiedLine;
									printf("Added: %s\n", cJSON_IsString(group) ? group->valuestring : "N/A");
									break;
								}
								else
								{
									printf("ELSE");
									sprintf(modifiedLine, "%s\n", line);
									modifiedLines[lineCount++] = modifiedLine;
								}
								free(nameGr);
								free(modifiedLine);
								line = strtok(NULL, "\n");
							}
							line = strtok(fileContent, "\n");
						}
						// Mở tệp để ghi lại
						FILE *file1 = fopen("../group.txt", "w");
						if (file1 == NULL)
						{
							fprintf(stderr, "Không thể mở tệp\n");
							return;
						}
						if (lineCount > 0 && modifiedLines != NULL)
						{
							for (int i = 0; i < lineCount; ++i)
							{
								// In ra nội dung để kiểm tra trước khi ghi vào tệp
								printf("Ghi: %s\n", modifiedLines[i]);
								fputs(modifiedLines[i], file1);
								fputc('\n', file1); // Thêm ký tự xuống dòng
								free(modifiedLines[i]);
							}
						}
						else
						{
							printf("Mang modifiedLines rong hoac lineCount <= 0\n");
						}

						fclose(file1); // Đóng tệp sau khi ghi dữ liệu

						// Giải phóng bộ nhớ
						free(fileContent);
						free(modifiedLines);

						// Đóng tệp
						fclose(file1);
					}
				}
				printf("\n");
				break;
			}
		}
	}

	// Giải phóng bộ nhớ
	cJSON_Delete(json);

	return;
}