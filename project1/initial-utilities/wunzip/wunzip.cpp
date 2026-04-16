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
static char print_buffer[4095];
static int print_buf_ind;

//take the letter and number of appearences, and write them into a different buffer which we will write out all at once
void Add_To_Output(int number, char letter)
{
    // add to the buffer
    for (int i = 0; i < number; i++)
    {
        // if the buffer is full/going to be, write out what we have and then reset the indexing
        if (print_buf_ind == 4095)
        {
            write(STDOUT_FILENO, print_buffer, 4095);
            print_buf_ind = 0;
        }

        print_buffer[print_buf_ind] = letter;
        print_buf_ind += 1;
    }
}

// Read through the compressed file
void File_Reader(int file_desc)
{
    ssize_t ret;

    // set buffer for compression
    static char buffer[4095];

    // keep track of what stuff we're printing
    int letter_count = 0;
    char letter_to_print;

    while((ret = read(file_desc, buffer, 4095)) > 0)
    {
        // format for every file: 
        // first 4 bytes = number
        // 1 byte after = letter

        for (ssize_t i = 0; i < ret; i +=5)
        {
            // get the number of letters to print (still need to use memcopy for bytes -> int)
            memcpy(&letter_count, buffer + i, 4);
            // get what letter to print
            letter_to_print = buffer[i + 4];

            Add_To_Output(letter_count, letter_to_print);
        }
    }
}

// argv[0] = ./wunzip
// argv[1] and beyond = test file
int main(int argc, char *argv[])
{
    int file_desc;

    // if only one argument, just ./wzip and no files
    if (argc == 1)
    {
        write(STDOUT_FILENO, "wunzip: file1 [file2 ...]\n", strlen("wunzip: file1 [file2 ...]\n"));
        return 1;
    }

    // if only 2 args, then just 1 file to unzip
    if (argc == 2)
    {
        file_desc = open(argv[1], O_RDONLY);

        // write error out if file doesn't exist
        if (file_desc < 0) {
            write(STDOUT_FILENO, "wunzip: cannot open file\n", strlen("wunzip: cannot open file\n"));
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
                write(STDOUT_FILENO, "wunzip: cannot open file\n", strlen("wunzip: cannot open file\n"));
                return 1;
            }

            // start unzipping the current file
            else {
                File_Reader(file_desc);
                close(file_desc);
            }
        }
    }

    // write out whatever's in the print_buffer
    write(STDOUT_FILENO, print_buffer, print_buf_ind);
    return 0;
}