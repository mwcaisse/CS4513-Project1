#ifndef __UTIL_H__
#define __UTIL_H__


#define TRASH_LOCATION "/home/mitchell/trash/"
#define TRASH_ENVAR "TRASH"


/** Determines if the string a starts with the string b,
	@param a cstring a
	@param b cstring b
	@return 1 if a starts with b, 0 otherwie
*/

int str_starts_with(const char* a, const char* b);

/** Fetches the trash location from the environment variable
	@return A cstring containing the trash location, or null 
		no environment variable for trash is set
*/
char* get_trash_location();


/** Determines if the specified file exists
	@param file cstring containing the path to the file to check
	@return 1 if the file exists, 0 otherwise
*/
int file_exists(char* file);

/** Takes the modified time of the file represneted by file and applies them to
		the file represented by the trash file
	@param file cstring containing the path to the file to clone the modified time of
	@param trash_file cstring containing the path to the file to apploy the moditied
		time to
	@return 0 if sucessful -1 otherwise
*/

int copy_file_time(char* file, char* trash_file);

/** Takes the file permisions of the file represneted by file, and applies them to
	the file represented by trash_file
	@param file The file to clone the permissions of
	@param trash_file The file to apply the permissions to
	@return 0 if sucessful -1 otherwise
*/

int copy_file_perms(char* file, char* trash_file);

#endif 