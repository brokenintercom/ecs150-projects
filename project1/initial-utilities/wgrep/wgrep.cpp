#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sstream>
#include <string> 
#include <cstring> 

using namespace std;

// function that looks through a line, checking if the search term is in there
bool Line_Searcher(string line, int length, string search_term)
{
    // length of our search term
    int word_size = search_term.size();

    // go through each character one by one until we hit all possible starts for the search term
    for (int k = 0; k <= length - word_size; k++){

        // if the current letter matches the starting letter of our search term, start letter-by-letter comparison
        if (line[k] == search_term[0]) {

            for (int l = 0; l < word_size; l++) {
                // If 2 letters are different, break out of the for loop and keep searching the line
                if (line[k + l] != search_term[l]) {
                    break;
                }
                // if we reached the last letter, that means all the letters were the same, meaning we return true
                if (l == word_size - 1) {
                    return true;
                }
            }
        }
    }

    return false;
}

// function that goes through the file, reading it until it hits a newline, then continuing
// Will call Line_Searcher for each line to decide whether to print it out or not
void Line_Reader(int fd, string search_term)
{
    // allocate space on the stack
    char buffer[4096];
    string curr_line; // turns out... std::string can update space for itself... kill me
    int ret; 

    // REMINDER: Ret is the number bytes we read from the file, but not the actual file content
    while ((ret = read(fd, buffer, 4096)) > 0) {

        // as we read stuff, go through each character in ret to check for newlines
        for (int j = 0; j < ret; j++)
        {
            // update curr_line as we go, adding the current letter to it
            curr_line += buffer[j];

            // if we read a \n, take all the letters in the curr_line and send it to line searcher
            if (buffer[j] == '\n')
            {
                int line_length = curr_line.length();

                // if the search term is in the line, print out the line
                // note: for strings in write(), have to use .c_str()
                if (Line_Searcher(curr_line, line_length, search_term)) {
                    write(STDOUT_FILENO, curr_line.c_str(), line_length);
                }

                // beginning a new line, clear out the last line we just did
                curr_line.clear();
            }
        }
    }
}

// argv[0] = ./wgrep
// argv[1] = search term
// argv[2] and beyond = files
int main(int argc, char *argv[])
{
    int file_desc;

    // if only 1 argument, then no search term or file (just ./wgrep given)
    if (argc == 1) {
        write(STDOUT_FILENO, "wgrep: searchterm [file ...]\n", strlen("wgrep: searchterm [file ...]\n"));
        return 1;
    }

    // if not 1, then at least search term given
    string search_term = argv[1]; // search term is always argv1

    // if argc = 2, then there's only wgrep and the search term
    if (argc == 2) {
        file_desc = STDIN_FILENO;

        // write error out if file doesn't exist
        if (file_desc < 0) {
            write(STDOUT_FILENO, "wzip: cannot open file\n", strlen("wzip: cannot open file\n"));
            return 1;
        }
        
        Line_Reader(file_desc, search_term);
        close(file_desc);
    }

    else {
        // start reading the files at arcv[2]
        for (int i = 2; i < argc; i++) {

            file_desc = open(argv[i], O_RDONLY);

            // write out error if the file is empty/broken, then return error
            if (file_desc < 0) {
                write(STDOUT_FILENO, "wgrep: cannot open file\n", strlen("wgrep: cannot open file\n"));
                return 1;
            }

            // otherwise, start checking the file's lines
            else {
                Line_Reader(file_desc, search_term);
                close(file_desc);
            }
        }
    }

    return 0;
}