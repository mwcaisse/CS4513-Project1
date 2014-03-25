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
		if (str_starts_with(argv[i], "-")) {
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
			closedir(dir);
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

void remove_file(const char* file, const char* trash) {

	char* trash_file = parse_trash_path(file, trash);
	
	if (rename(file, trash_file)) {
		if (errno == EXDEV) {
			remove_file_partition(file, trash_file);
		}
		else {
			if (!force) 	perror("couldnt delete file ");
		}
	}

	free(trash_file);
	
}

/** Sends the specifeid directory to the trash,
	@param dir A cstring containing the path to the directory
	@param trash A cstring containing the full path to the trash directory
*/

void remove_dir(const char* dir, const char* trash) {

	char* trash_dir = parse_trash_path(dir, trash);
	
	if (rename(dir, trash_dir)) {
		if (errno == EXDEV) {
			remove_dir_partition(dir, trash_dir);
		}
		else {
			if (!force) 	perror("couldnt delete directory ");
		}
	}
}

/** Removes the specified file, which exists on a different parition than trash directory
	@param file The path to the file to remove
	@param trash_file The file to create in trash directory
*/

void remove_file_partition(const char* file, const char* trash_file) {
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

/** Removes a directory that is on a different partition than the trash directory
	@param dir cstring containing the path to the directory
	@param trash_dir cstring containing the path to the trash directory
	@return 0 if sucessful -1 otherwise, sets errno
*/

int remove_dir_partition(const char* dir, const char* trash_dir) {

	//create a directory
	if (mkdir(trash_dir, 0777)) {
		return -1;
	}

	char* dir_tmp = (char*) malloc(strlen(dir) + 1);
	strncpy(dir_tmp, dir, strlen(dir) + 1) ;
	
	char* dir_name = basename(dir_tmp);
	
	//create the new path to the trash directory
	int new_trash_len = strlen(trash_dir) + strlen(dir_name) + 2;
	char* new_trash = (char*) malloc(new_trash_len);
	strncpy(new_trash, trash_dir, new_trash_len);
	strncat(new_trash, "/", 2);
	strncat(new_trash, dir_name, new_trash_len);

	DIR* dirc = opendir(dir);
	if (dir == NULL) {
		return -1;
	}
	else {
		struct dirent* entry; // a directory entry
		//craft our path, then loop through entrie
		
		while ( (entry = readdir(dirc)) != NULL) {			
			//woo we have the name, lets get the path.	
			int path_len = strlen(dir) + strlen(entry->d_name) + 3;
			char* new_path = (char*) malloc(path_len);
			
			strncpy(new_path, dir, path_len);
			strncat(new_path, "/", 2);
			strncat(new_path, entry->d_name, path_len);
			
			if (is_dir(new_path)) {
				remove_dir_partition(new_path, new_trash);
			}
			else {
				remove_file_partition(new_path, new_trash);
			}
			
			//free the new path		
			free(new_path);
		}
	}
	
	//free the trash path + close the file
	free(new_trash);
	closedir(dirc);
	rmdir(dir); // remove the directory
}


/** Returns the file extension to add to the file before adding it to the trash
		ie. if file helloworld.ls already exists, it will return .1
	@param file A cstring containing the file to parse extension for
	@return The extension to append to file_name before copying it to trash,
		should be freed after use
*/

char* get_trash_file_extension(const char* file) {
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

/** Parses the path of the file to create in the trash folder for the specified file
	@param path to the file to parse the path for
	@param path to the trash directory
	@return a cstring containing the path of the file in the trash, or NULL if error,
		this pointer should be freed after use
*/

char* parse_trash_path(const char* file, const char* trash) {
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
	
	free(file_path);
	free(trash_file_extension);
	
	return trash_file;
}


