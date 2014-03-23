#ifndef __RM_H__
#define __RM_H__

#define MAX_TRASH_EXTENSION_LEN (5)

/** Prints the usage of RM

*/

void print_usage();

/** Sends the specified file to the trash
	@param file A cstring containing the path to the file
	@param trash A cstring containing the full path to the trash directory
*/

void remove_file(char* file, char* trash);

/** Returns the file extension to add to the file before adding it to the trash
		ie. if file helloworld.ls already exists, it will return .1
	@param file A cstring containing the file to parse extension for
	@return The extension to append to file_name before copying it to trash,
		should be freed after use
*/

char* get_trash_file_extension(char* file);


#endif
