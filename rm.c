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
				if (force) {
					if (delete_file(argv[i])) {
						perror("unable to delete file");
					}
				}
				else {				
					if (move_file(argv[i], trash_location)) {
						perror("unable to move file to trash");
					}
				}
			}
			else {
				perror("error removing file");
			}
		}
		else {
			closedir(dir);
			if (recurse) {
				if (force) {
					if (delete_dir(argv[i])) {
						perror("unable to delete directory");
					}
				}
				else {
					if (move_dir(argv[i], trash_location)) {
						perror("unable to move directory to trash");
					}
				}
			}
			else {
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

