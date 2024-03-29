#include "FileManagementServer.h"

void fileManager(int conn_sock, char *path, int permission)
{
	char recv_data[BUFF_SIZE];
	int bytes_received;

	// int status;
	int choice;

	// start conversation
	do
	{
		printf("file manager");
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
			printf("recv file\n");
			recv_file(conn_sock, path); // loop receives files from client
			printf("recv file end\n");
			break;
		case 2:
			printf("send file\n");
			send_file(conn_sock, path); // loop send files to client
			printf("send file end\n");
			break;
		case 3:
			printf("create folder\n");
			createFolder(conn_sock, path);
			printf("create folder end\n");
			break;
		case 4:
			printf("get list file\n");
			getListFile(conn_sock, path);
			printf("get list file end\n");
			break;
		case 5:
			printf("rename file\n");
			renameFile(conn_sock, path, permission);
			printf("rename file end\n");
			break;
		case 6:
			printf("copy file\n");
			renameFile(conn_sock, path, permission);
			printf("copy file end\n");
			break;
		case 7:
			printf("delete file");
			deleteFile(conn_sock, path, permission);
			printf("delete file end\n");
			break;
		default:
			break;
		}

	} while (choice > 0 && choice < 8);

	return;
}

int recv_file(int conn_sock, char *dir_name)
{
	struct stat st;
	int size_file;
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;
	char *tok;
	FILE *fp = NULL;
	char file_path[BUFF_SIZE + 128];
	int nLeft, idx;

	// check if directory exists, if not mkdir
	if (stat(dir_name, &st) == -1)
	{
		char *tmp_path = (char *)malloc(sizeof(char) * 512);
		getcwd(tmp_path, 512);
		char *folder = strdup(dir_name);
		tok = strtok(folder, "/");
		while (tok != NULL)
		{
			mkdir(tok, 0700); // config permissions by changing 2nd argument
			chdir(tok);		  // chdir ~ cd
			tok = strtok(NULL, "/");
		}
		chdir(tmp_path);
		free(tmp_path);
		free(folder);
	}

	// receives file name
	bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0);
	if (bytes_received < 0)
	{
		perror("Error: ");
		return -1; // meet error, aborted
	}
	else
		recv_data[bytes_received] = '\0'; // check with client send format

	if (recv_data[0] == '\0')
	{
		printf("Receiving data from client end. Exiting.\n");
		return 1;
	}

	if (strcmp(recv_data, MSG_CLOSE) == 0)
	{ // file not found on client
		printf("No file found on client. Transmission aborted.\n");
		return -1;
	}

	// check if file exists, if not create new one, else return error
	// already at destination folder
	// echo to client
	sprintf(file_path, "%s/%s", dir_name, recv_data);
	printf("%s\n", file_path);
	if (stat(file_path, &st) == -1)
	{ // file does not exist
		fp = fopen(file_path, "wb");
		if (fp == NULL)
		{
			printf("File path error\n");
			fclose(fp);
			return -1;
		}
		bytes_sent = send(conn_sock, MSG_RECV, strlen(MSG_RECV), 0); // echo that received file name and no duplicate file on server
		if (bytes_sent < 0)
		{
			perror("\nError: ");
			fclose(fp);
			return -1; // meet error, aborted
		}
	}
	else
	{
		printf("Duplicate file.\n");
		bytes_sent = send(conn_sock, MSG_DUP_FILE, strlen(MSG_DUP_FILE), 0); // echo that received file name and duplicate file on server
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}
		return -1;
	}

	printf("File name: %s\n", recv_data);
	bzero(recv_data, bytes_received); // empty buffer

	// receives file size
	bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0);
	if (bytes_received < 0)
	{
		perror("Error: ");
		fclose(fp);
		return -1; // meet error, aborted
	}
	else
		recv_data[bytes_received] = '\0'; // check with client send format

	size_file = atoi(recv_data);

	printf("File size: %s\n", recv_data);
	bzero(recv_data, bytes_received); // empty buffer

	nLeft = size_file % BUFF_SIZE; // cuz file size is not divisible by BUFF_SIZE
	int loop_size = size_file;

	while (loop_size > 0)
	{
		idx = 0; // reset idx

		while (nLeft > 0)
		{
			bytes_received = recv(conn_sock, &recv_data[idx], nLeft, 0); // read at missing data index
			if (bytes_received <= 0)
			{
				// Error handler
				printf("Connection closed. Trying again.\n");
			}
			idx += bytes_received; // if larger then socket size
			nLeft -= bytes_received;
		}

		fwrite(recv_data, 1, idx, fp); // idx is the real length of recv_data
		bzero(recv_data, sizeof(recv_data));
		loop_size -= BUFF_SIZE; // decrease unfinished bytes   		-
		nLeft = BUFF_SIZE;		// reset nLeft
	}

	// sucessful block
	fclose(fp);
	return 0;
}

int send_file(int conn_sock, char *dir_name)
{ // char dir_name[] = "Downloadable files";
	printf("send file");
	struct stat st;
	char buff[BUFF_SIZE];
	char sendbuff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	char *tok;
	FILE *fp;
	int nLeft, idx;
	char *file_path = NULL;
	char *file_name = NULL;
	off_t file_size = 0;
	char file_size_str[65];
	size_t result = 0;

	// check if directory exists, if not mkdir
	if (stat(dir_name, &st) == -1)
	{
		char *tmp_path = (char *)malloc(sizeof(char) * 512);
		getcwd(tmp_path, 512);
		char *folder = strdup(dir_name);
		tok = strtok(folder, "/");
		while (tok != NULL)
		{
			mkdir(tok, 0700); // config permissions by changing 2nd argument
			chdir(tok);		  // chdir ~ cd
			tok = strtok(NULL, "/");
		}
		chdir(tmp_path);
		free(tmp_path);
		free(folder);
	}

	file_path = get_file_path(conn_sock, dir_name); // need to get file path function here
	if (file_path == NULL)
	{
		// error occurred
		printf("No file path specified.\n");
		return -1; // change it for future
	}
	// if error occurred, file permissions deny
	if (stat(file_path, &st) == -1 || !S_ISREG(st.st_mode))
	{ // Not exists or not a regular file
		fprintf(stderr, "Error: File not found..\n");
		bytes_sent = send(conn_sock, MSG_CLOSE, strlen(MSG_CLOSE), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
			return -1;
		}
		return -1;
	}

	file_name = extract_file_name(file_path);
	if (file_name == NULL)
	{
		printf("Error: Client cancel transfer or error occurred.\n");
		return -1;
	}

	printf("Uploading file to client: %s\n", file_name);
	bytes_sent = send(conn_sock, file_name, strlen(file_name), 0);
	if (bytes_sent < 0)
	{
		perror("\nError: ");
		return -1;
	}

	// confirm client received file name and check file status on server side
	bytes_received = recv(conn_sock, buff, BUFF_SIZE - 1, 0);
	if (bytes_received < 0)
	{
		perror("Error: ");
		return -1;
	}
	else
		buff[bytes_received] = '\0';

	if (strcmp(buff, MSG_DUP_FILE) == 0)
	{ // file was found on server, duplicate file
		printf("%s\n", buff);
		return -1;
	}

	bzero(buff, sizeof(buff));

	file_size = st.st_size;
	sprintf(file_size_str, "%lu", file_size);
	printf("File size is %s\n", file_size_str);
	bytes_sent = send(conn_sock, file_size_str, strlen(file_size_str), 0);
	if (bytes_sent < 0)
	{
		perror("\nError: ");
		return -1;
	}

	// open file and send data
	if ((fp = fopen(file_path, "rb")) == NULL)
	{ // need send error message
		fprintf(stderr, "Open file error.\n");
		exit(1);
	}
	int loop_size = file_size;
	nLeft = file_size % BUFF_SIZE; // cuz file size is not divisible by BUFF_SIZE

	while (loop_size > 0)
	{
		idx = 0;

		result += fread(sendbuff, 1, nLeft, fp); // use fread instead of fgets because fgets stop reading if newline is read
		while (nLeft > 0)
		{
			// Assume s is a valid, connected stream socket
			bytes_sent = send(conn_sock, &sendbuff[idx], nLeft, 0);
			if (bytes_sent <= 0)
			{
				// Error handler
				printf("Connection closed.Trying again.\n");
			}
			nLeft -= bytes_sent;
			idx += bytes_sent;
		}

		bzero(sendbuff, sizeof(sendbuff));
		loop_size -= BUFF_SIZE; // decrease unfinished bytes
		nLeft = BUFF_SIZE;		// reset nLeft
	}

	if (result != file_size)
	{
		printf("Error reading file.\n");
		return -1;
	}

	// clean
	fclose(fp);
	free(file_name);
	free(file_path);
	return 0;
}

int deleteFile(int conn_sock, char *file_name, int permission)
{
	struct stat st;
	char send_data[BUFF_SIZE];
	int bytes_sent;
	char *file_path = NULL;

	if (permission != 1)
	{
		bytes_sent = send(conn_sock, MSG_CLOSE, strlen(MSG_CLOSE), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}
		return -1;
	}

	file_path = get_file_path(conn_sock, file_name); // Get the file path
	if (file_path == NULL)
	{
		// Error occurred
		printf("No file path specified.\n");
		return -1;
	}

	// Check if the file or folder exists
	if (stat(file_path, &st) == -1)
	{
		fprintf(stderr, "Error: File or folder not found.\n");
		bytes_sent = send(conn_sock, MSG_CLOSE, strlen(MSG_CLOSE), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}

		free(file_path);
		return -1;
	}

	// Delete the file or folder
	if (S_ISDIR(st.st_mode))
	{
		// It's a directory, use rmdir to remove it
		if (rmdir(file_path) != 0)
		{
			perror("\nError: ");
			free(file_path);
			return -1;
		}
	}
	else
	{
		// It's a file, use unlink to remove it
		if (unlink(file_path) != 0)
		{
			perror("\nError: ");
			free(file_path);
			return -1;
		}
	}

	// Send success message
	snprintf(send_data, BUFF_SIZE, "Deleted: %s", file_name);
	bytes_sent = send(conn_sock, send_data, strlen(send_data), 0);
	if (bytes_sent < 0)
	{
		perror("\nError: ");
		free(file_path);
		return -1;
	}

	free(file_path);
	return 0;
}

int createFolder(int conn_sock, char *dir_name)
{
	struct stat st;
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;
	char *folder_path = NULL;

	folder_path = get_file_path(conn_sock, dir_name); // can also to get folder path
	if (folder_path == NULL)
	{
		// error occurred
		printf("No folder path specified.\n");
		return -1; // change it for future
	}

	bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); // receive folder name
	if (bytes_received < 0)
	{
		perror("Error: ");
		return -1; // meet error, aborted
	}
	else
		recv_data[bytes_received] = '\0';

	// if not choose a folder but a file
	if (stat(folder_path, &st) == -1 || !S_ISDIR(st.st_mode))
	{ // Not exists or not a directory
		fprintf(stderr, "Error: Folder not found or you not choose a folder.\n");
		bytes_sent = send(conn_sock, MSG_CLOSE, strlen(MSG_CLOSE), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}
		return -1;
	}

	// create sub folder
	char *tmp_path = (char *)malloc(sizeof(char) * 512);
	getcwd(tmp_path, 512);
	chdir(folder_path);
	if (stat(recv_data, &st) != -1)
	{
		bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");

			chdir(tmp_path);
			free(tmp_path);
			free(folder_path);

			return -1;
		}
	}
	else
	{
		mkdir(recv_data, 0700);
		bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");

			chdir(tmp_path);
			free(tmp_path);
			free(folder_path);

			return -1;
		}
	}

	chdir(tmp_path);
	free(tmp_path);
	//

	free(folder_path);
	return 0;
}

char *extract_file_name(char *file_path)
{
	int i;
	int n = strlen(file_path);
	char *file_name;
	for (i = n - 1; i >= 0; --i)
	{
		if (file_path[i] == '/')
			break;
	}

	if (i == 0) // current directory so that no '/'
		return file_path;

	file_name = (char *)malloc((n - i) * sizeof(char));
	memcpy(file_name, &file_path[i + 1], n - i);

	return file_name;
}

char *get_file_path(int conn_sock, char *dir_name)
{
	struct stat st;
	DIR *mydir = NULL;			  // pointer to directory
	struct dirent *myfile = NULL; // struct to get directory information
	// create temporary directory for file path
	char *cur_path = (char *)malloc(sizeof(char) * 512);
	strcpy(cur_path, dir_name);
	// contains info
	char *file_path = (char *)malloc(sizeof(char) * 512);
	char file_name[128];
	// for sending signal
	char buff[BUFF_SIZE];
	char tmp[512];
	int bytes_sent, bytes_received;
	char *tmp_path = (char *)malloc(sizeof(char) * 512);

	printf("Starting sending file list...\n");

	while (1)
	{
		// opendir() returns a pointer of DIR type.
		getcwd(tmp_path, 512);
		printf("File pointer now at %s\n", tmp_path);
		mydir = opendir(cur_path);

		if (mydir == NULL) // opendir returns NULL if couldn't open directory
		{
			sprintf(buff, "Could not open \"%s\" directory on server\n", cur_path);
			bytes_sent = send(conn_sock, buff, strlen(buff), 0);
			if (bytes_sent < 0)
			{
				perror("\nError: ");
				return NULL;
			}
			return NULL;
		}
		else
		{
			bytes_sent = send(conn_sock, MSG_ACCEPT, strlen(MSG_ACCEPT), 0);
			if (bytes_sent < 0)
			{
				perror("\nError: ");
				return NULL;
			}
		}

		memset(buff, 0, sizeof(buff));
		sprintf(buff, "List of files: \n");

		while ((myfile = readdir(mydir)) != NULL)
		{
			sprintf(file_path, "%s/%s", cur_path, myfile->d_name);
			stat(file_path, &st);
			sprintf(tmp, "%zu", st.st_size);
			strcat(buff, tmp);
			sprintf(tmp, " %s\n", myfile->d_name);
			strcat(buff, tmp);
		}
		closedir(mydir);

		bytes_sent = send(conn_sock, buff, strlen(buff), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
			return NULL;
		}
		// user choose file/folder
		bytes_received = recv(conn_sock, file_name, 128, 0);
		if (bytes_received < 0)
		{
			perror("\nError: ");
			return NULL;
		}
		else
			file_name[bytes_received] = '\0';

		if (strcmp(file_name, MSG_CLOSE) == 0)
		{ /// cancel download
			printf("Receive signal close.\n");
			return NULL;
		}

		if (strcmp(file_name, "..") == 0)
		{
			if (strcmp(cur_path, dir_name) != 0)
			{
				int n = strlen(cur_path);
				for (int i = n - 1; i > 0; --i)
				{
					if (cur_path[i] != '/')
						cur_path[i] = '\0';
					else
					{
						cur_path[i] = '\0';
						break;
					}
				}
			}
			else
			{
				printf("Invalid path. Stay in current directory.\n");
			}
		}
		else if (strcmp(file_name, ".") == 0)
		{ // this for choosing a folder
			printf("Choose %s directory.\n", cur_path);
			bytes_sent = send(conn_sock, MSG_ACCEPT, strlen(MSG_ACCEPT), 0);
			if (bytes_sent < 0)
			{
				perror("\nError: ");
				return NULL;
			}
			return cur_path; // return folder path
		}
		else
		{
			strcat(cur_path, "/");
			strcat(cur_path, file_name);
		}

		stat(cur_path, &st);
		if (S_ISREG(st.st_mode))
		{
			bytes_sent = send(conn_sock, MSG_ACCEPT, strlen(MSG_ACCEPT), 0);
			if (bytes_sent < 0)
			{
				perror("\nError: ");
				return NULL;
			}
			return cur_path;
		}
		else if (S_ISDIR(st.st_mode))
		{
			memset(buff, 0, sizeof(buff));
			sprintf(buff, "Redirect to \"%s\" ...\n", file_name);
			bytes_sent = send(conn_sock, buff, strlen(buff), 0);
			if (bytes_sent < 0)
			{
				perror("\nError: ");
				return NULL;
			}
			continue;
		}
	}
}

void getListFile(int conn_sock, char *dir_name)
{
	DIR *dir;
	struct dirent *entry;
	struct stat file_info;

	char list_buffer[BUFF_SIZE];
	memset(list_buffer, 0, sizeof(list_buffer));

	dir = opendir(dir_name);

	if (dir == NULL)
	{
		perror("Error opening directory");
		return;
	}

	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}

		char entry_path[BUFF_SIZE];
		snprintf(entry_path, sizeof(entry_path), "%s/%s", dir_name, entry->d_name);

		if (stat(entry_path, &file_info) == -1)
		{
			perror("Error getting file information");
			continue;
		}

		if (S_ISDIR(file_info.st_mode))
		{
			// If it is a directory, append "(DIR)" to the entry name
			snprintf(list_buffer + strlen(list_buffer), sizeof(list_buffer) - strlen(list_buffer),
					 "%s (DIR)\n", entry->d_name);
		}
		else if (S_ISREG(file_info.st_mode))
		{
			// If it is a regular file, append "(FILE)" to the entry name
			snprintf(list_buffer + strlen(list_buffer), sizeof(list_buffer) - strlen(list_buffer),
					 "%s (FILE)\n", entry->d_name);
		}
		// Add other conditions for different types of files if needed
	}

	closedir(dir);
	int bytes_sent = send(conn_sock, list_buffer, strlen(list_buffer), 0);
	if (bytes_sent < 0)
	{
		perror("\nError sending list: ");
	}
}

int renameFile(int conn_sock, char *dir_name, int permission)
{
	struct stat st;
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;
	char *file_path = NULL;

	if (permission != 1)
	{
		bytes_sent = send(conn_sock, MSG_CLOSE, strlen(MSG_CLOSE), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}
		return -1;
	}

	file_path = get_file_path(conn_sock, dir_name); // get file path
	if (file_path == NULL)
	{
		// error occurred
		printf("No file path specified.\n");
		return -1; // change it for future
	}

	bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); // receive new file name
	if (bytes_received < 0)
	{
		perror("Error: ");
		return -1; // meet error, aborted
	}
	else
		recv_data[bytes_received] = '\0';

	// if error occurred, file permissions deny
	if (stat(file_path, &st) == -1)
	{ // File not exists
		fprintf(stderr, "Error: File not found.\n");
		bytes_sent = send(conn_sock, MSG_CLOSE, strlen(MSG_CLOSE), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}
		return -1;
	}

	char *new_path = (char *)malloc(sizeof(char) * 512);
	getcwd(new_path, 512);

	if (rename(file_path, recv_data) != 0)
	{
		// Error occurred while renaming the file
		perror("Error: ");
		bytes_sent = send(conn_sock, MSG_ERROR, strlen(MSG_ERROR), 0);
		if (bytes_sent < 0)
		{
			perror("\nError: ");
		}

		free(new_path);
		free(file_path);

		return -1;
	}

	// Send the new file name to the client
	bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0);
	if (bytes_sent < 0)
	{
		perror("\nError: ");
		free(new_path);
		free(file_path);
		return -1;
	}

	free(new_path);
	free(file_path);
	return 0;
}
