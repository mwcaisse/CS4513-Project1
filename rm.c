#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>


#include "rm.h"
#include "util.h"

/** ignore nonexistent files and arguments */
int force = 0;
/** remove directories and thier contents recursivly */
int recurse = 0;

int main(int argc,  char* argv[]) {

	//parse the arguments
	char c;
	while ((c = getopt (argc, argv, "fhr")) != EOF) {
		switch (c) {
		
		case 'f':
			force = 1;
			break;
		case 'h':
			print_usage();
			return;
		case 'r':
			recurse = 1;
			break;
		default:
			printf("other item... %c \n", c);
			break;
		}
	}
	
	// get the trash location
	
	char* trash_location = get_trash_location();
	if (trash_location == NULL) {
		printf("TRASH environment variable is not set. Set it to the"
			" location of the trash to use rm \n");
		return 1;
	}
	
	// parse the files
	int i;
	for (i=1; i < argc; i++) {	
		if (str_starts_with(argv[i] "-")) {
			//this is an argument not a file.
			continue;
		}
		
		//else check if this is a directory
	
		DIR* dir = opendir(argv[i]);
		if (dir == NULL) {
			if (errno = ENOENT) {
				//dir is null, and errno is ENOENT, argv is a file
				remove_file(argv[i], trash_location);
			}
			else {
			if (!force) 	perror("error removing file: ");
			}
		}
		else {
			if (recurse) {
				remove_dir(argv[i], trash_location);
			}
			else if (!force) {
				printf("Error %s is a directory \n", argv[i]);
			}	
		}
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

void remove_file(const char* file, char* trash) {
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
	
	if (rename(file, trash_file)) {
		if (errno == EXDEV) {
			remove_file_partition(file, trash_file);
		}
		else {
			if (!force) 	perror("couldnt delete file ");
		}
	}
	
	free(file_path);
	free(trash_file);
	free(trash_file_extension);
	
}

/** Sends the specifeid directory to the trash,
	@param dir A cstring containing the path to the directory
	@param trash A cstring containing the full path to the trash directory
*/

void remove_dir(const char* dir, char* trash) {

}

/** Removes the specified file, which exists on a different parition than trash directory
	@param file The path to the file to remove
	@param trash_file The file to create in trash directory
*/

void remove_file_partition(char* file, char* trash_file) {
	FILE* file_src = fopen(file, "r");
	//check to make sure we can open the file
	if (file_src < 0) {
		if (!force) 	printf("Unable to open file %s, couldn't delete\n", file);	
		return;
	}
	
	FILE* file_dst = fopen(trash_file, "w+");
	//check to make sure we were able to create the file
	if (file_dst < 0) {
		fclose(file_src);
		if (!force) 	printf("Unable to create file %s, couldn't delete\n", trash_file);
		return;
	}
	
	int fd_src = fileno(file_src);
	int fd_dst = fileno(file_dst);
	
	//the buffer to copy the files
	char buffer[FILE_BUFFER];
	
	int num_read;
	while ( (num_read = read(fd_src, buffer, FILE_BUFFER)) > 0) {
		if (write(fd_dst, buffer, num_read) != num_read) {
			if (!force) 	printf("error writing file \n");
		}
	}
	if (num_read == -1) {
		if (!force) 	perror("error reading file:");
		return;
	}

	//file permissions
	if (copy_file_perms(file, trash_file)) {
		if (!force) 	perror("error copying file permissions: ");
		return;
	}
	//file access times
	if (copy_file_time(file, trash_file)) {
		if (!force) 	perror("error copying access times: ");
		return;
	}
	
	//delete the original file from the directory tree
	if (unlink(file)) {
		if (!force) 	perror("error unlinking file: ");
		return;
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
