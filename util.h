#ifndef __UTIL_H__
#define __UTIL_H__

#define FILE_BUFFER (8192)
#define TRASH_ENVAR "TRASH"
#define MAX_TRASH_EXTENSION_LEN (5)


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
int file_exists(const char* file);

/** Takes the modified time of the file represneted by file and applies them to
		the file represented by the trash file
	@param file cstring containing the path to the file to clone the modified time of
	@param trash_file cstring containing the path to the file to apploy the moditied
		time to
	@return 0 if sucessful -1 otherwise
*/

int copy_file_time(const char* file, const char* trash_file);

/** Takes the file permisions of the file represneted by file, and applies them to
	the file represented by trash_file
	@param file The file to clone the permissions of
	@param trash_file The file to apply the permissions to
	@return 0 if sucessful -1 otherwise
*/

int copy_file_perms(const char* file, const char* trash_file);


/** Determines if the item in the path is a directory or not
	@param path cstring containing the path to the directory to test
	@return 1 if it is a dir, 0 otherwise, or if an error occured
*/

int is_dir(const char* path);

/** Movies the specified file, to the specified directory, if a file with the same
		name already exists in the directory, a number is appended to the end.
		ie if file already exists in the folder, the file is moved as file.001
	@param file A cstring containing the path to the file
	@param dir A cstring containing the full path to the directory to move the file to
	@return 0 if successful, -1 otherwise
*/

int move_file(const char* file, const char* dir);

/** Deletes the specifeid file from disk
	@param file A cstring containing the path to the file to delete
	@return 0 if successful, -1 otherwise, sets errno
*/

int delete_file(const char* file);

/** Deletes the specified directory from disk
	@oaram dir A cstring containing the path of the directory to delete
	@return 0 if sucessful, -1 otherwise
*/

int delete_dir(const char* dir);

/** Deletes the contents of a specified directory
	@param dir A cstring containing the path of the directory whoose contents to delete
	@return 0 if successful, -1 otherwise
*/

int delete_dir_contents(const char* dir);

/** Moves the specified directory to the other directory
	@param src A cstring containing the path to the directory to move
	@param dst A cstring containing the path to the destination directory
	@return 0 if successful, -1 otherwise
*/

int move_dir(const char* src, const char* dst);

/** Moves a file accross partitions
	@param file The path to the file to move
	@param dst_file The path of the file to move the file to
	@return 0 if successful -1 otherwise
*/

int move_file_partition(const char* file, const char* dst_file);

/** Moves a directory cross partitions
	@param src A cstring containing the directory to move
	@param dst A cstring containing the path of the destination of the directory, including
		the directories name
	@return 0 if sucessful -1 otherwise, sets errno
*/

int move_dir_partition(const char* src, const char* dst);

/** Appends the directory name given onto the end of the specified path
	@param dir_name The name of the directory to append to the end of the path
	@param path The path to append the dir_name onto
	@return A cstring containing the appended path, should free after use,
		or NULL if unsucessful
*/

char* append_to_path(const char* dir_name, const char* path);


/** Returns the file extension to add to the file before adding it to the trash
		ie. if file helloworld.ls already exists, it will return .1
	@param file A cstring containing the file to parse extension for
	@return The extension to append to file_name before copying it to trash,
		should be freed after use
*/

char* get_trash_file_extension(const char* file);

/** Parses the path of the file to create in the trash folder for the specified file
	@param path to the file to parse the path for
	@param path to the trash directory
	@return a cstring containing the path of the file in the trash, or NULL if error,
		this pointer should be freed after use
*/

char* parse_trash_path(const char* file, const char* trash);


#endif 
