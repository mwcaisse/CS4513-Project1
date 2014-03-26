#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>


#include "util.h"

/** Determines if the string a starts with the string b,
	@param a cstring a
	@param b cstring b
	@return 1 if a starts with b, 0 otherwie
*/

int str_starts_with(const char* a, const char* b) {
	return strncmp(a, b, strlen(b)) == 0;
}

/** Fetches the trash location from the environment variable
	@return A cstring containing the trash location, or null 
		no environment variable for trash is set
*/
char* get_trash_location() {
	return getenv(TRASH_ENVAR);
} 

/** Determines if the specified file exists
	@param file cstring containing the path to the file to check
	@return 1 if the file exists, 0 otherwise
*/
int file_exists(const char* file) {
	return access(file, F_OK) == 0;
}

/** Takes the file permisions of the file represneted by file, and applies them to
	the file represented by trash_file
	@param file The file to clone the permissions of
	@param trash_file The file to apply the permissions to
	@return 0 if sucessful -1 otherwise and set errno
*/

int copy_file_perms(const char* file, const char* trash_file) {
	struct stat file_stats;
	if (stat(file, &file_stats)) {
		return -1;
	}	
	if (chmod(trash_file, file_stats.st_mode)) {
		return -1;
	}
	return 0;
}

/** Takes the modified time of the file represneted by file and applies them to
		the file represented by the trash file
	@param file cstring containing the path to the file to clone the modified time of
	@param trash_file cstring containing the path to the file to apploy the moditied
		time to
	@return 0 if sucessful -1 otherwise and set errno
*/

int copy_file_time(const char* file, const char* trash_file) {
	struct stat file_stats; // file statistics
	struct utimbuf times; // file times
	if (stat(file, &file_stats)) {
		return -1;
	}	
	//set the times in the time struct
	times.actime = file_stats.st_atime;
	times.modtime = file_stats.st_mtime;
	
	if (utime(trash_file, &times)) {
		return -1;
	}
	return 0;
}

/** Determines if the item in the path is a directory or not
	@param path cstring containing the path to the directory to test
	@return 1 if it is a dir, 0 otherwise, or if an error occured
*/

int is_dir(const char* path) {
	int ret = 0;
	DIR* dir = opendir(path);
	if (dir) {
		closedir(dir);
		ret = 1;
	}

	return ret;

}


/** Movies the specified file, to the specified directory, if a file with the same
		name already exists in the directory, a number is appended to the end.
		ie if file already exists in the folder, the file is moved as file.001
	@param file A cstring containing the path to the file
	@param dir A cstring containing the full path to the directory to move the file to
	@return 0 if successful, -1 otherwise
*/

int move_file(const char* file, const char* dir) {

	char* moved_file = parse_trash_path(file, dir);
	
	if (rename(file, moved_file)) {
		if (errno == EXDEV) {
			return move_file_partition(file, moved_file);
		}
		else {
			return -1;
		}
	}
	free(moved_file);	
	return 0;
	
}

/** Deletes the specifeid file from disk
	@param file A cstring containing the path to the file to delete
	@return 0 if successful, -1 otherwise, sets errno
*/

int delete_file(const char* file) {
	return unlink(file);
}

/** Deletes the specified directory from disk
	@oaram dir A cstring containing the path of the directory to delete
	@return 0 if sucessful, -1 otherwise
*/

int delete_dir(const char* dir) {
	if (delete_dir_contents(dir)) {
		return -1;
	}
	return rmdir(dir);
	
}

/** Deletes the contents of a specified directory
	@param dir A cstring containing the path of the directory whoose contents to delete
	@return 0 if successful, -1 otherwise
*/

int delete_dir_contents(const char* dir) {
	DIR* dir_ptr = opendir(dir);
	if (dir_ptr == NULL) {
		return -1;
	}
	else {
		struct dirent* entry; // a directory entry
		//craft our path, then loop through entrie
		
		while ( (entry = readdir(dir_ptr)) != NULL) {		
		
			if (strncmp(entry->d_name, ".", 1) == 0 
				|| strncmp(entry->d_name, "..", 1) == 0) {
				
				//entry is . or .. continue
				continue;	
			}
			
			//woo we have the name, lets get the path.	
			int path_len = strlen(dir) + strlen(entry->d_name) + 3;
			char* new_path = (char*) malloc(path_len);
			
			strncpy(new_path, dir, path_len);
			strncat(new_path, "/", 2);
			strncat(new_path, entry->d_name, path_len);
			
			if (is_dir(new_path)) {			
				if (delete_dir(new_path)) {
					perror("error deleting directory");
				}
			}
			else {							
				if (delete_file(new_path)) {
					perror("error deleting file");
				}
			}			
			//free the new path		
			free(new_path);
		}
	}
}

/** Moves the specified directory to the other directory
	@param src A cstring containing the path to the directory to move
	@param dst A cstring containing the path to the destination directory
	@return 0 if successful, -1 otherwise
*/

int move_dir(const char* src, const char* dst) {

	char* dst_dir = parse_trash_path(src, dst);
	
	if (rename(src, dst_dir)) {
		if (errno == EXDEV) {
			int ret = move_dir_partition(src, dst_dir);
			free(dst_dir);
			return ret;
		}
		else {
			free(dst_dir);
			return -1;
		}
	}
	free(dst_dir);
	return 0;
}

/** Moves a file accross partitions
	@param file The path to the file to move
	@param dst_file The path of the file to move the file to
	@return 0 if successful -1 otherwise
*/

int move_file_partition(const char* file, const char* dst_file) {
	FILE* file_src = fopen(file, "r");
	//check to make sure we can open the file
	if (file_src == NULL) {
		return -1;
	}
	
	FILE* file_dst = fopen(dst_file, "w+");
	//check to make sure we were able to create the file
	if (file_dst == NULL) {
		fclose(file_src);
		return -1;
	}
	
	int ret = 0;
	
	int fd_src = fileno(file_src);
	int fd_dst = fileno(file_dst);
	
	//the buffer to copy the files
	char buffer[FILE_BUFFER];
	
	int num_read;
	while ( (num_read = read(fd_src, buffer, FILE_BUFFER)) > 0) {
		write(fd_dst, buffer, num_read);
	}
	if (num_read == -1) {
		ret = -1;
		goto exit;
	}

	//file permissions
	if (copy_file_perms(file, dst_file)) {
		ret = -1;
		goto exit;
	}
	//file access times
	if (copy_file_time(file, dst_file)) {
		ret = -1;
		goto exit;
	}
	
	//delete the original file from the directory tree
	if (unlink(file)) {
		ret = -1;
		goto exit;
	}
	
	
	exit:
		fclose(file_src);
		fclose(file_dst);	
		return ret;
}

/** Moves a directory cross partitions
	@param src A cstring containing the directory to move
	@param dst A cstring containing the path of the destination of the directory, including
		the directories name
	@return 0 if sucessful -1 otherwise, sets errno
*/

int move_dir_partition(const char* src, const char* dst) {

	//create a directory
	if (mkdir(dst, 0777)) {
		return -1;
	}

	DIR* dir = opendir(src);
	if (dir == NULL) {
		return -1;
	}
	else {
		struct dirent* entry; // a directory entry
		//craft our path, then loop through entrie
		
		while ( (entry = readdir(dir)) != NULL) {		
		
			if (strncmp(entry->d_name, ".", 1) == 0 
				|| strncmp(entry->d_name, "..", 1) == 0) {
				
				//entry is . or .. continue
				continue;	
			}
			
			//woo we have the name, lets get the path.	
			int path_len = strlen(src) + strlen(entry->d_name) + 3;
			char* new_path = (char*) malloc(path_len);
			
			strncpy(new_path, src, path_len);
			strncat(new_path, "/", 2);
			strncat(new_path, entry->d_name, path_len);
			
			if (is_dir(new_path)) {			
				//create the new trash directory
				char* new_trash = append_to_path(entry->d_name, dst);			
				if (move_dir_partition(new_path, new_trash)) {
					perror("couldnt delete directory");
				}
				free(new_trash);
			}
			else {							
				char* trash_file = parse_trash_path(new_path, dst);				
				if (move_file_partition(new_path, trash_file)) {
					perror("couldnt remove file");
					printf("file new_path |%s| \n trash_file |%s| \n", new_path, trash_file);
				}
				free(trash_file);
			}
			
			//free the new path		
			free(new_path);
		}
	}
	
	//close the directory
	closedir(dir);
	return rmdir(src); // remove the directory
}

/** Appends the directory name given onto the end of the specified path
	@param dir_name The name of the directory to append to the end of the path
	@param path The path to append the dir_name onto
	@return A cstring containing the appended path, should free after use,
		or NULL if unsucessful
*/

char* append_to_path(const char* dir_name, const char* path) {
	//create the new path to the trash directory
	int new_trash_len = strlen(path) + strlen(dir_name) + 2;
	char* new_trash = (char*) malloc(new_trash_len);
	strncpy(new_trash, path, new_trash_len);
	strncat(new_trash, "/", 2);
	strncat(new_trash, dir_name, new_trash_len);
	strncat(new_trash, "/", 2);
	return new_trash;				
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
		+ MAX_TRASH_EXTENSION_LEN + 4;
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




