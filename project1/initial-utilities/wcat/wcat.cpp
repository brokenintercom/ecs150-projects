#include <iostream>

#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sstream>

using namespace std;

// function called for printing out contents of file
void File_print(int fd) {

    // once we've gotten the file, start reading the file
    char buffer[4096]; 
    int ret; 

    // file_desc = the file we're reading
    // buffer: pointer to the space we're allocating on the staack
    // 4096: max amount of bytes we're reading/allocated
    while((ret = read(fd, buffer, 4096)) > 0) {
        // print out the stuff while we're reading it
        write(STDOUT_FILENO, buffer, ret);
    }
}

// argc: how many arguments you have
// *argv[]: character pointers, holds the input/arguments
int main(int argc, char *argv[]) {

    // file descriptor, an index to an entry in the process' table of open fd's
    // basically, gives us access to the file and what we use when wanting to access it
    int file_desc;

    // if you only have one argument, that means it's just wcat, send it to the input stream
    if (argc == 1) {
        file_desc = STDIN_FILENO;
        File_print(file_desc);
    }

    // if you have multiple arguments (2nd argument would be file name), try to open each one
    // O_RDONLY makes it so that the program can't accidently write into the file
    else {

        for (int i = 1; i < argc; i++)
        {
            // open the current file we're looking at
            file_desc = open(argv[i], O_RDONLY); 

            // write out error if the file is like, empty/broken, then return error
            if (file_desc < 0) {
                write(STDOUT_FILENO, "wcat: cannot open file\n", 23);
                return 1;
            }

            // call File_print to print out the text in the file, then close it
            else {
                File_print(file_desc);
                close (file_desc);
            }
        }
    }

    return 0;
}