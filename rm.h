#ifndef __RM_H__
#define __RM_H__

#define MAX_TRASH_EXTENSION_LEN (5)
#define FILE_BUFFER (8192)

/** Prints the usage of RM

*/

void print_usage();

/** Sends the specified file to the trash
	@param file A cstring containing the path to the file
	@param trash A cstring containing the full path to the trash directory
*/

void remove_file(const char* file, const char* trash);

/** Sends the specifeid directory to the trash,
	@param dir A cstring containing the path to the directory
	@param trash A cstring containing the full path to the trash directory
*/

void remove_dir(const char* dir, const char* trash);

/** Parses the path of the file to create in the trash folder for the specified file
	@param path to the file to parse the path for
	@param path to the trash directory
	@return a cstring containing the path of the file in the trash, or NULL if error,
		this pointer should be freed after use
*/

char* parse_trash_path(const char* file, const char* trash);

/** Returns the file extension to add to the file before adding it to the trash
		ie. if file helloworld.ls already exists, it will return .1
	@param file A cstring containing the file to parse extension for
	@return The extension to append to file_name before copying it to trash,
		should be freed after use
*/

char* get_trash_file_extension(const char* file);

/** Removes the specified file, which exists on a different parition than trash directory
	@param file The path to the file to remove
	@param trash_file The file to create in trash directory
*/

void remove_file_partition(const char* file, const char* trash_file);

/** Removes a directory that is on a different partition than the trash directory
	@param dir cstring containing the path to the directory
	@param trash_dir cstring containing the path to the trash directory
	@return 0 if sucessful -1 otherwise
*/

int remove_dir_partition(const char* dir, const char* trash_dir);

#endif
