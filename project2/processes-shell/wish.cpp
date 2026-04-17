#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <queue>
#include <cstring>

#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/select.h>

using namespace std;

// error message
char error_message[30] = "An error has occurred\n";

// check if we can access the path/cmd that the user wants to do, and return the exectuable path
string Path_checker(string path_input, vector<string> active_paths)
{
    // access(const char *path, mode)
    // here, the mode X_OK doesn't just check if it exists, but if it's executable
    for (size_t i = 0; i < active_paths.size(); i++)
    {
        // should have something like /bin/ls
        string path = active_paths[i] + '/' + path_input;
        const char *full_path = path.c_str();

        if (access(full_path, X_OK) == 0)
        {
            return path;
        }
    }

    // if none of the paths worked, it's an error; return empty 
    return "";
}

// function that takes input and fixes it for proper useage
vector<string> Input_Reader(string input)
{
    stringstream input_stream(input);
    vector<string> input_vector;
    string cmd;

    while(getline(input_stream, cmd, '&'))
    {
        input_vector.push_back(cmd);
    }

    return input_vector;
}

// argc = 2
// argv[1] = ./wish
// argv[2] = the file with all the inputs being typed in
int main(int argc, char *argv[])
{
    bool exiting = false;
    bool interactive_mode;
    vector<string> commands;
    vector<string> active_paths = {"/bin"}; // active paths that built in commands like can use (defined by path)

    istream* input_mode = &cin; // istream represents input stream, so can use either cin or ifstream for input depending on mode
    ifstream file; // file input, can assign input to this address instead of cin address 
    string input;

    // batch mode input = ./wish tests/1.in, so arc = 2
    if (argc == 2)
    {
        file.open(argv[1]);
        if (!file.is_open()) // if file didn't open/doesn't exist, raise error
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        input_mode = &file;
        interactive_mode = false;
    }

    // shouldn't have more than 2 args
    else if (argc > 2)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

//-------------------------------- GETTING INPUT AND SETTIGN UP COMMANDS --------------------------------//

    // run in a while loop until user wants to stop/exit
    while (!exiting)
    {
        // start with clearing previous cmds from the vector
        commands.clear();

        if (interactive_mode)
        {
            write(STDOUT_FILENO, "wish> ", strlen("wish> "));
        }

        if (!getline(*input_mode, input))
        {
            exit(0);
        }

        commands = Input_Reader(input); // ex of what we should have: <ls -la /tmp, cd>

//-------------------------------- SETTING UP ARGS FOR EACH COMMAND --------------------------------//

        // vector for each child we run
        vector<pid_t> child_forks;

        // for each command, split into arguments
        // note: commands.size() returns "size_type", so typecast to int
        for (size_t i = 0; i < commands.size(); i++)
        {
            // string stream good for splitting words, handling whitespace and tab
            // TO DO: CHANGE TO BE BETTER SINCE > DOESN'T NEED WHHITESPACE; ls>output
            stringstream split_args(commands[i]);
            vector<string> split_args_vec;
            string arg;

            // clean out the vector
            split_args_vec.clear();

            // feed the separated args into a vector for proper ussage
            // result: "ls /no/such/file" -> <ls, /no/such/file>
            while(split_args >> arg)
            {
                split_args_vec.push_back(arg);
            }

//-------------------------------- BUILT-IN COMMANDS --------------------------------//
            // check if the first command is empty. If yes, go to next command
            if (split_args_vec.empty()) continue;

            // check what command we're doing. If built in, do correlating function. If not, then we run a fork
            if (split_args_vec[0] == "exit")
            {
                if (split_args_vec.size() > 1)
                {
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                }

                else 
                { 
                    exit(0);
                }
            }

            // change directories using chdir(), argument supplied by user
            // should only have one argument, so size == 2
            else if (split_args_vec[0] == "cd")
            {
                if (split_args_vec.size() != 2) 
                {
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                }

                else 
                {
                    // use chdir(const char *path) to change from current director to the director the user inputs
                    if (chdir((char*)split_args_vec[1].c_str()) != 0)
                    {
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                    }
                }
            }

            else if (split_args_vec[0] == "path")
            {
                active_paths.clear();

                if (split_args_vec.size() > 1)
                {
                    for (size_t i = 1; i < split_args_vec.size(); i++)
                    {
                        active_paths.push_back(split_args_vec[i]);
                    }
                }
            }

//-------------------------------- SHELL COMMANDS --------------------------------//

            else 
            {
                // get the path command that the user want to do
                string path = split_args_vec[0];
                string full_path = Path_checker(path, active_paths);
                
                // if the path is valid, do stuff
                if (!full_path.empty())
                {
                    // run a fork to execv the command; because execv "replaces" current program running, doing it in a copy of this program allows us to keep running this "main" program
                    pid_t pid = fork();

                    if (pid == 0) // in the child process
                    {
                        // first, get the args for the command we're doing into a char *argv[]
                        vector<char*> argv_vec;
                        for (size_t i = 0; i < split_args_vec.size(); i++)
                        {
                            // convert strings in the vector into char*
                            argv_vec.push_back((char*)split_args_vec[i].c_str());
                        }
                        
                        // add null pointer at the end of the vector
                        argv_vec.push_back(nullptr);

                        // use .data() so the vector can be treated as an array/pointers
                        execv(full_path.c_str(), argv_vec.data());

                        // if the execv above returns, that means an error happened, so print message then exit
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                        exit(1);
                    }


                    else if (pid > 0) // in parent process, store child process for later
                    {
                        child_forks.push_back(pid); 
                    }
                }

                // if not valid directory, return error
                else
                {
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                }
            }
        }

        // do a wait for each child/non-built in command
        for (size_t child = 0; child < child_forks.size(); child++)
        {
            // waitpid(pid, status, options)
            // status = encoded bitfield that tells us why/how a child process changes its state
            // options = set behavior of the wait; 0 makes it wait for any child process who matches the pid, not just any child process
            waitpid(child_forks[child], NULL, 0);
        }
    }

    return 0;
}
