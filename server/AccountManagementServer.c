#include "AccountManagementServer.h"

void readFromFile(Account *root, FILE *file)
{
	Account *tmp = root;

	// Lấy kích thước của tệp
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Đọc nội dung của tệp vào một chuỗi
	char *json_content_root = (char *)malloc(file_size + 1);
	fread(json_content_root, 1, file_size, file);

	// Phân tích tệp JSON thành một đối tượng cJSON
	cJSON *json_array = cJSON_Parse(json_content_root);
	free(json_content_root);

	// Lặp qua mỗi đối tượng trong mảng JSON
	cJSON *jsonObject = NULL;
	cJSON_ArrayForEach(jsonObject, json_array)
	{
		// Lấy giá trị các trường từ đối tượng
		const char *username = cJSON_GetObjectItemCaseSensitive(jsonObject, "username")->valuestring;
		const char *password = cJSON_GetObjectItemCaseSensitive(jsonObject, "password")->valuestring;
		int status = cJSON_GetObjectItemCaseSensitive(jsonObject, "status")->valueint;

		// Lưu thông tin của đối tượng
		Account *newAccount = (Account *)malloc(sizeof(Account));
		newAccount->username = strdup(username);
		newAccount->password = strdup(password);
		newAccount->status = status;
		newAccount->next = NULL;

		tmp->next = newAccount;
		tmp = tmp->next;
	}

	// Giải phóng bộ nhớ
	cJSON_Delete(json_array);

	return;
}

void registerAccount(Account *root, int conn_sock, FILE *db)
{ // done
	char username[255];
	char password[64];
	int isDuplicate = 0;
	Account *tmp;

	int bytes_sent, bytes_received;

	// receive username from client
	while (1)
	{
		isDuplicate = 0; // reset isDuplicate

		bzero(username, 255);
		bytes_received = recv(conn_sock, username, 256, 0);
		if (bytes_received <= 0)
		{
			fprintf(stderr, "Failed to receive username from client. Try again.\n");
			return;
		}
		else
			username[bytes_received] = '\0';
		// receive null or there's already an user
		if (strcmp(username, MSG_FALSE) == 0)
		{
			printf("Cancel registration.\n");
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
			if (strcmp(tmp->username, username) == 0)
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
			printf("Account existed.\n");
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

	// receive password from client
	bzero(password, 64);
	bytes_received = recv(conn_sock, password, 64, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive password from client. Try again.\n");
		return;
	}
	else
		password[bytes_received] = '\0';

	Account *newAccount = (Account *)malloc(sizeof(Account));

	if (newAccount == NULL)
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
		newAccount->username = strdup(username);
		newAccount->password = strdup(password);
		newAccount->status = ACTIVE;
		newAccount->next = NULL;
		tmp->next = newAccount;

		printf("%s\n", newAccount->username);
		printf("%s\n", newAccount->password);

		// Mở file account.json để đọc dữ liệu
		fseek(db, 0, SEEK_END);
		long file_size = ftell(db);
		fseek(db, 0, SEEK_SET);
		char *json_content = (char *)malloc(file_size + 1);
		fread(json_content, 1, file_size, db);

		// Phân tích tệp JSON thành đối tượng cJSON
		cJSON *jsonArray = cJSON_Parse(json_content);
		free(json_content);
		fclose(db);

		// Tạo đối tượng cjson
		cJSON *json = cJSON_CreateObject();
		cJSON *array = cJSON_CreateArray();
		cJSON_AddStringToObject(json, "username", newAccount->username);
		cJSON_AddStringToObject(json, "password", newAccount->password);
		cJSON_AddNumberToObject(json, "status", newAccount->status);
		cJSON_AddItemToObject(json, "invitation", array);
		// Thêm đối tượng mới vào mảng đã phân tích từ tệp JSON
		cJSON_AddItemToArray(jsonArray, json);
		// Mở file và replace nội dung
		FILE *file_write = fopen("../account.json", "w");
		if (file_write == NULL)
		{
			fprintf(stderr, "Cannot open target file.");
			exit(EXIT_FAILURE);
		}
		char *json_string = cJSON_Print(jsonArray);
		int bytes_written = fprintf(file_write, "%s", json_string);
		fclose(file_write);

		// Giải phóng bộ nhớ
		cJSON_Delete(jsonArray);
		free(json_string);
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

void signInAccount(Account *root, int conn_sock, FILE *db, int *isLoggedIn)
{
	char username[255];
	char password[64];
	int bytes_sent, bytes_received;
	int status;
	Account *tmp;

	bzero(username, 255);
	bytes_received = recv(conn_sock, username, 256, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive username from client. Try again.\n");
		return;
	}
	else
		username[bytes_received] = '\0';

	// receive null or there's already have an user
	if (strcmp(username, MSG_FALSE) == 0)
	{
		fprintf(stderr, "Already logged in.\n");
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
		if (strcmp(tmp->username, username) == 0)
		{
			printf("Found.\n");
			if (tmp->status == ACTIVE)
			{
				bytes_sent = send(conn_sock, MSG_TRUE, strlen(MSG_TRUE), 0); // inform that username is valid
				if (bytes_sent <= 0)
				{
					fprintf(stderr, "Failed to send signal to client. Try again.\n");
					return;
				}

				for (int index = 0; index < 3; index++)
				{
					bzero(password, 64);
					bytes_received = recv(conn_sock, password, 64, 0);
					if (bytes_received <= 0)
					{
						fprintf(stderr, "Failed to receive password from client. Try again.\n");
						return;
					}
					else
						password[bytes_received] = '\0';
					// username and password are matched.
					if (strcmp(tmp->password, password) == 0)
					{
						// generate sessionID for client
						bytes_sent = send(conn_sock, username, strlen(username), 0); // temporarily use username instead of sessionID
						if (bytes_sent <= 0)
						{
							fprintf(stderr, "Failed to send signal to client. Try again.\n");
							return;
						}
						// change status of account node in account tree (block until current user log out)
						// we don't need to block user here while create multi process
						*isLoggedIn = 1;
						return;
					}
					else
					{
						fprintf(stderr, "Password is incorrect.\n");
						bytes_sent = send(conn_sock, MSG_FALSE, strlen(MSG_FALSE), 0); // request client re-enter password
						if (bytes_sent <= 0)
						{
							fprintf(stderr, "Failed to send signal to client. Try again.\n");
							return;
						}
					}
				}

				// block account in tree and in file, not release
				tmp->status = BLOCKED;
				fseek(db, 0, SEEK_SET);
				while (!feof(db))
				{
					fscanf(db, "%s %s %d\n", username, password, &status);
					if (strcmp(tmp->username, username) == 0)
					{
						fseek(db, -2, SEEK_CUR); // skip \n then 1 from backwards
						fputc('0', db);			 // change from 1 to 0
						fseek(db, 0, SEEK_CUR);	 // to meet requirement of ISO standard.
						return;
					}
				}
			}
			else
			{
				fprintf(stderr, "Account is blocked.\n");
				bytes_sent = send(conn_sock, MSG_BLOCKED, strlen(MSG_BLOCKED), 0);
				if (bytes_sent <= 0)
				{
					fprintf(stderr, "Failed to send signal to client. Try again.\n");
				}
				return;
			}
		}
		tmp = tmp->next;
	}

	printf("Cannot find account.\n");
	bytes_sent = send(conn_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to send signal to client. Try again.\n");
	}
	return;
}

void searchAccount(Account *root, int conn_sock)
{
	char username[255];
	Account *tmp;
	int bytes_sent, bytes_received;

	bytes_received = recv(conn_sock, username, 256, 0);
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to verify username. Try again.\n");
		return;
	}
	else
		username[bytes_received] = '\0';

	// receive null
	if (strcmp(username, MSG_FALSE) == 0)
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
		if (strcmp(tmp->username, username) == 0)
		{
			if (tmp->status != 0)
			{
				printf("Account is active\n");
				bytes_sent = send(conn_sock, MSG_TRUE, strlen(MSG_TRUE), 0);
				if (bytes_sent <= 0)
				{
					fprintf(stderr, "Failed to send signal to client. Try again.\n");
					return;
				}
			}
			else
			{
				printf("Account is blocked\n");
				bytes_sent = send(conn_sock, MSG_FALSE, strlen(MSG_FALSE), 0);
				if (bytes_sent <= 0)
				{
					fprintf(stderr, "Failed to send signal to client. Try again.\n");
					return;
				}
			}
			return;
		}
		tmp = tmp->next;
	}

	printf("Cannot find account.\n");
	bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
	if (bytes_sent <= 0)
	{
		fprintf(stderr, "Failed to send signal to client. Try again.\n");
		return;
	}
}

void signOutAccount(Account *root, int conn_sock, int *isLoggedIn)
{ // not done ỷet
	char username[255];
	int bytes_sent, bytes_received;
	Account *tmp;

	// fprintf(stderr, "bug0\n");
	// temporarily use username instead of sessionID
	memset(username, 0, 255);
	fprintf(stderr, "User log out\n");
	bytes_received = recv(conn_sock, username, 255, 0);
	// fprintf(stderr, "bug1");
	if (bytes_received <= 0)
	{
		fprintf(stderr, "Failed to receive sessionID from client. Try again.\n");
		return;
	}
	else
		username[bytes_received] = '\0';
	// fprintf(stderr, "bug2");
	if (strcmp(username, MSG_FALSE) == 0) // user not sign in so cannot log out
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
		if (strcmp(tmp->username, username) == 0)
		{
			printf("Goodbye %s\n", username);
			*isLoggedIn = 0; // log out
			bytes_sent = send(conn_sock, MSG_TRUE, strlen(MSG_TRUE), 0);
			if (bytes_sent <= 0)
			{
				fprintf(stderr, "Failed to send signal to client. Try again.\n");
				return;
			}
			return;
		}
		tmp = tmp->next;
	}
}

void printFile(FILE *fp)
{
	char username[255];
	char password[64];
	int status;

	fseek(fp, 0, SEEK_SET);
	while (!feof(fp))
	{
		fscanf(fp, "%s %s %d\n", username, password, &status);
		printf("%s %s %d\n", username, password, status);
	}

	return;
}

void freeAccount(Account *root)
{
	Account *tmp = root->next;
	Account *guard = tmp;
	if (tmp == NULL)
		return; // empty linked list
	while (tmp != NULL)
	{
		guard = tmp->next;
		free(tmp->username);
		free(tmp->password);
		free(tmp);
		tmp = guard;
	}
}

void homepage(int sockfd)
{
	char recv_data[BUFF_SIZE];
	int bytes_received;

	// int status;
	int choice;

	Group *root = (Group *)calloc(1, sizeof(Group));

	FILE *db = fopen("../group.txt", "r+");
	if (db == NULL)
	{
		fprintf(stderr, "cannot open group list file\n");
		exit(2);
	}
	readGroupInfo(root, db);

	// start conversation
	do
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
		choice = atoi(recv_data);

		switch (choice)
		{
		case 1:
			createGroup(root, sockfd, db);
			break;
		case 2:
			getIntoGroup(root, sockfd, db);
			break;
		case 3:
			searchGroup(root, sockfd);
			break;
		case 4:
			getListGroup(root, sockfd);
			break;
		default: // end process
			freeGroupList(root);
			break;
		}

	} while (choice > 0 && choice < 5);

	return;
}
