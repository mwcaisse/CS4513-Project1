#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>


#include "dump.h"
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

	//deletes the contents of the trash directory
	delete_dir_contents(trash_location);

}

/** Prints the usage of DV

*/

void print_usage() {
	printf("Usage: \n");
	printf("\t ./dump [-h] \n");
}

 