#include <iostream>
#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sstream>

using namespace std;

// function called for printing out contents of file
void File_print(int fd) {

}

// argc: how many arguments you have
// *argv[]: character pointers, holds the input/arguments
int main(int argc, char *argv[]) {

    vector<int> fdVec;
    fdVec.push_back(STDIN_FILENO);

    for (int idx = 1; idx < argc; idx++)
    {
        int fx = open(argv[idx], O_RDONLY);
        fdVec.push_back(fx);
    }

    for (int idx = 0; idx < fdVec.size(); idx++)
    {
        if (fork() == 0) {
            // child
            int fd = fdVec[idx];
            char buffer[4096];
            int ret;

            while ((ret = read(fd, buffer, sizeof(buffer))) > 0)
            {
                write(STDOUT_FILENO, buffer, ret);
            }

            close(fd);

            // MAKE SURE TO RETURN: if we don't return/exit, the next loop we'd end up having 4 processes, 2 parents that called a fork creating a child
            return 0;
        }

        // else {
        //     // it's the parent, wait till a process is done... however because we make it wait, we basically did the ssame thing we did before, as we're waiting for the child to finish
        //     wait(NULL);
        // }
    }

    // SOLUTION: MOVE wait to AFTER the for loop
    // fdVec = number of children, so use that to call wait for each child process
    for (int idx = 0; idx < fdVec.size(); idx++)
    {
        wait(NULL);
    }

    cout << "all done!" << endl;
    return 0;
}