#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fstream>
#include <string> 
#include <cstring>

using namespace std;

// static variables that are updated as we go through each file
// prevents "reset" when moving between files, will now work to count letters across multiple files
static char curr_char = 0;
static int curr_counter = 0;
static bool started = false;

// set buffer for compression
static char buffer[4096];
static int buf_index = 0; //keep track of where to put compression stuff in the buffer 

//take the letter and number of appearences, and write them out to output as a 5-byte entry
void File_Compressor(int number, char letter)
{
    // check if the buffer is full (or going to be). If yes, write it out and reset the indexing. otherwise, write to the buffer 
    if (buf_index + 5 > 4096)
    {
        write(STDOUT_FILENO, buffer, buf_index);
        buf_index = 0;
    }

    // add the number to the buffer (address where we want it to go), and then add 4 to account for 4 byte size
    // NOTE: can't directly place the int into char buffer, so use memcpy to "copy n bytes from &number to memory area buffer"
    memcpy(buffer + buf_index, &number, 4);
    buf_index += 4;

    // same idea with letter, but 1 byte to index
    buffer[buf_index] = letter;
    buf_index += 1;
}

// Read through the file
void File_Reader(int file_desc)
{
    // allocate space for the read
    char buffer[4096];
    int ret;

    while((ret = read(file_desc, buffer, 4096)) > 0)
    {
        for (int i = 0; i < ret; i++)
        {
            // assign the first letter as curr_char when first starting and THEN move to i = 2;
            if (!started)
            {
                curr_char = buffer[i];
                curr_counter = 1;
                started = true;
                continue;
            }
            
            //if curr_char == the current letter we're looking at, add 1 to the counter
            else if (curr_char == buffer[i])
            {
                curr_counter += 1;
            }

            // if the two characters we're looking at are different, compress the letter, and update trackers as we see new letter
            else if (curr_char != buffer[i])
            {
                File_Compressor(curr_counter, curr_char);

                curr_char = buffer[i];
                curr_counter = 1;
            }
        }
    }
}

// argv[0] = ./wzip
// argv[1] and beyond = test file
int main(int argc, char *argv[])
{
    int file_desc;

    // if only one argument, just ./wzip and no files
    if (argc == 1)
    {
        write(STDOUT_FILENO, "wzip: file1 [file2 ...]\n", strlen("wzip: file1 [file2 ...]\n"));
        return 1;
    }

    // if only 2 args, then just 1 file to compress
    if (argc == 2)
    {
        file_desc = open(argv[1], O_RDONLY);

        // write error out if file doesn't exist
        if (file_desc < 0) {
            write(STDOUT_FILENO, "wzip: cannot open file\n", strlen("wzip: cannot open file\n"));
            return 1;
        }

        File_Reader(file_desc);
        close(file_desc);
    }

    else {
        // go through each file in argv
        for (int i = 1; i < argc; i++)
        {
            file_desc = open(argv[i], O_RDONLY);

            // write error out if file doesn't exist
            if (file_desc < 0) {
                write(STDOUT_FILENO, "wzip: cannot open file\n", strlen("wzip: cannot open file\n"));
                return 1;
            }

            // start compressign the current file
            else {
                File_Reader(file_desc);
                close(file_desc);
            }
        }
    }

    // after we're done reading through all the files, print out whatever's left (use started so that it only does this if it ran File_Reader/has files to read)
    if (started)
    {
        File_Compressor(curr_counter, curr_char);
        write(STDOUT_FILENO, buffer, buf_index);
    }

    return 0;
}