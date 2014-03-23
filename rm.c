#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#include "rm.h"
#include "util.h"


int main(int argc,  char* argv[]) {

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
			printf("File to delete is on different partition than trash -.- \n");
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
		printf("read:%d:%s \n", num_read, buffer);
		if (write(fd_dst, buffer, num_read) != num_read) {
			printf("Amount written different than amount read \n");
		}
	}
	if (num_read == -1) {
		perror("error reading file:");
	}

	
	fclose(file_src);
	fclose(file_dst);
	

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
