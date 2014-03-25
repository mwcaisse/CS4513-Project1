#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

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
