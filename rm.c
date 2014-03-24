#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>

#include "rm.h"
#include "util.h"


int main(int argc,  char* argv[]) {
/*
	char c;
	while ((c = getopt (argc, argv, "fhr")) != EOF) {
		switch (c) {
		
		case 'f':
			printf("force was entered as an option \n");
			break;
		case 'h':
			printf("help was entered as an option \n");
			break;
		case 'r':
			printf("recurse over directories \n");
			break;
		default:
			printf("other item... %c \n", c);
			break;
		}
	}

	printf("ARGC: %d \n", argc);

	return 0;*/

	if (argc != 2) {
		print_usage();
		return 1;
	}
	
	char* trash_location = get_trash_location();
	if (trash_location == NULL) {
		printf("TRASH environment variable is not set. Set it to the"
			" location of the trash to use rm \n");
		return 1;
	}
	DIR* dir = opendir(argv[1]);
	if (dir == NULL) {
		if (errno = ENOENT) {
			//dir is null, and errno is ENOENT, argv is a file
			remove_file(argv[1], TRASH_LOCATION);
		}
		else {
			perror("error removing file: ");
		}
	}
	else {
		//argv is a directory
		printf("cannot delete %s, it is a directory \n", argv[1]);
	}

}

/** Prints the usage of RM

*/

void print_usage() {
	printf("Usage: \n");
	printf("\t rm file \n");
}

/** Sends the specified file to the trash
	@param file A cstring containing the path to the file
	@param trash A cstring containing the full path to the trash directory
*/

void remove_file(char* file, char* trash) {

	//copy the file path into a local var to use basename
	char* file_path = (char*) malloc(strlen(file) + 1);
	strncpy(file_path, file, strlen(file) + 1);
	
	char* file_name = basename(file_path);
	
	//allocate sapce for trash file, + 2 for / and null terminator
	int trash_file_len = strlen(trash) + strlen(file_name) 
		+ MAX_TRASH_EXTENSION_LEN + 2;
	char* trash_file = (char*) malloc(trash_file_len);
	

	
	//construct the trash file string
	strncpy(trash_file, trash, trash_file_len);
	strncat(trash_file, "/", 2);
	strncat(trash_file, file_name, trash_file_len);

	//get the trash file extension
	char* trash_file_extension = get_trash_file_extension(trash_file);
	strncat(trash_file, trash_file_extension, trash_file_len);
	
	if (rename(file, trash_file) == 0) {
		printf("Sucessfully sent file %s to trash \n", file);
	}
	else {
		if (errno == EXDEV) {
			remove_file_partition(file, trash_file);
		}
		else {
			printf("Error sending file %s to trash \n", file);
			perror("\t ");
		}
	}
	
	free(file_path);
	free(trash_file);
	free(trash_file_extension);
	
}

/** Removes the specified file, which exists on a different parition than trash directory
	@param file The path to the file to remove
	@param trash_file The file to create in trash directory
*/

void remove_file_partition(char* file, char* trash_file) {
	FILE* file_src = fopen(file, "r");
	//check to make sure we can open the file
	if (file_src < 0) {
		printf("Unable to open file %s, couldn't delete\n", file);	
		return;
	}
	
	FILE* file_dst = fopen(trash_file, "w+");
	//check to make sure we were able to create the file
	if (file_dst < 0) {
		fclose(file_src);
		printf("Unable to create file %s, couldn't delete\n", trash_file);
		return;
	}
	
	int fd_src = fileno(file_src);
	int fd_dst = fileno(file_dst);
	
	//the buffer to copy the files
	char buffer[FILE_BUFFER];
	
	int num_read;
	while ( (num_read = read(fd_src, buffer, FILE_BUFFER)) > 0) {
		if (write(fd_dst, buffer, num_read) != num_read) {
			printf("Amount written different than amount read \n");
		}
	}
	if (num_read == -1) {
		perror("error reading file:");
	}

	//file permissions
	copy_file_perms(file, trash_file);
	//file access times
	copy_file_time(file, trash_file);
	
	fclose(file_src);
	fclose(file_dst);
	

}


/** Takes the file permisions of the file represneted by file, and applies them to
	the file represented by trash_file
	@param file The file to clone the permissions of
	@param trash_file The file to apply the permissions to
	@return 0 if sucessful -1 otherwise
*/

int copy_file_perms(char* file, char* trash_file) {
	struct stat file_stats;
	if (stat(file, &file_stats)) {
		perror("error getting file permissions: ");
		return -1;
	}
	
	if (chmod(trash_file, file_stats.st_mode)) {
	perror("error setting file permissions: ");
		return -1;
	}
	return 0;
}

/** Takes the modified time of the file represneted by file and applies them to
		the file represented by the trash file
	@param file cstring containing the path to the file to clone the modified time of
	@param trash_file cstring containing the path to the file to apploy the moditied
		time to
	@return 0 if sucessful -1 otherwise
*/

int copy_file_time(char* file, char* trash_file) {
	struct stat file_stats; // file statistics
	struct utimbuf times; // file times
	if (stat(file, &file_stats)) {
		perror("error getting file times: ");
		return -1;
	}
	
	//set the times in the time struct
	times.actime = file_stats.st_atime;
	times.modtime = file_stats.st_mtime;
	
	if (utime(trash_file, &times)) {
	perror("error setting file times: ");
		return -1;
	}
	return 0;
}

/** Returns the file extension to add to the file before adding it to the trash
		ie. if file helloworld.ls already exists, it will return .1
	@param file A cstring containing the file to parse extension for
	@return The extension to append to file_name before copying it to trash,
		should be freed after use
*/

char* get_trash_file_extension(char* file) {
	int test_file_len = strlen(file) + MAX_TRASH_EXTENSION_LEN;
	char* test_file = (char*) malloc(test_file_len + 1);
	
	char* cur_ext = (char*) malloc(MAX_TRASH_EXTENSION_LEN + 1);
	strncpy(cur_ext, "", MAX_TRASH_EXTENSION_LEN);
	
	int ext_num = 0;
	
	strncpy(test_file, file, test_file_len);
	strncat(test_file, cur_ext, test_file_len);
	
	while (file_exists(test_file)) {
		ext_num ++;
		snprintf(cur_ext, MAX_TRASH_EXTENSION_LEN, ".%03d", ext_num);
		
		//copy over the extension into the file name
		strncpy(test_file, file, test_file_len);
		strncat(test_file, cur_ext, test_file_len);
	}
	
	free (test_file);
	return cur_ext;
	
}
