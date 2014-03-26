#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>


#include "dv.h"
#include "util.h"


int main(int argc,  char* argv[]) {

	//parse the arguments
	char c;
	while ((c = getopt (argc, argv, "h")) != EOF) {
		print_usage();
		return;
	}
	
	// get the trash location
	
	char* trash_location = get_trash_location();
	if (trash_location == NULL) {
		printf("TRASH environment variable is not set. Set it to the"
			" location of the trash to use \n");
		return -1;
	}
	
	char cur_dir[CUR_DIR_BUFFER_SIZE];
	if (getcwd(cur_dir, CUR_DIR_BUFFER_SIZE) == NULL) {
		perror("couldnt get cwd");
		return -1;
	}
	
	// parse the files
	int i;
	for (i=1; i < argc; i++) {	
		if (str_starts_with(argv[i], "-")) {
			//this is an argument not a file.
			continue;
		}
		
		int file_restore_len = strlen(argv[i]) + strlen(trash_location) + 2;
		char* file_restore = (char*) malloc(file_restore_len);
		
		strncpy(file_restore, trash_location, file_restore_len);
		strncat(file_restore, "/", 2);
		strncat(file_restore, argv[i], file_restore_len);
		
		//check if the file is a directory
		if (is_dir(file_restore)) {
			if (move_dir(file_restore, cur_dir)) {
				perror("unable to restore directory");
			}
		}
		else {
			if (move_file(file_restore, cur_dir)) {
				perror("unable to restole file");
			}
		}		
		free(file_restore);
		
	}

}

/** Prints the usage of DV

*/

void print_usage() {
	printf("Usage: \n");
	printf("\t ./dv [-h] file [files..] \n");
}

