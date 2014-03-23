#ifndef __UTIL_H__
#define __UTIL_H__


#define TRASH_LOCATION "/home/mitchell/trash/"
#define TRASH_ENVAR "TRASH"


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

#endif 