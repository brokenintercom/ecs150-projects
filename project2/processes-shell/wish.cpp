#include <iostream>
#include <sstream>

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

// check if we can access the path/cmd that the user wants to do, and return the exectuable path
string Path_checker(string path_input)
{
    string bin_input = "/bin/" + path_input;
    string usr_input = "/usr/bin/" + path_input;

    const char *bin = bin_input.c_str();
    const char *usr = usr_input.c_str();

    // access(const char *path, mode)
    // here, the mode X_OK doesn't just check if it exists, but if it's executable
    if (access(bin, X_OK) == 0)
    {
        return bin;
    }

    if (access(usr, X_OK) == 0)
    {
        return usr;
    }

    else { return ""; }
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
    vector<string> commands;

    // run in a while loop until user wants to stop/exit
    while (!exiting)
    {
        
//-------------------------------- GETTING INPUT AND SETTIGN UP COMMANDS --------------------------------//

        string input;

        // print out "wish>" for command line prompt, will need to do again after entering input
        write(STDOUT_FILENO, "wish> ", strlen("wish> "));

        // start with clearing previous cmds from the vector
        commands.clear();

        // read one line of cmds: ls -la /tmp & cd
        if (!getline(cin, input))
        {
            // if user inputs/hits eof, call exit(0)
            exit(0);
        }

        commands = Input_Reader(input); // ex of what we should have: <ls -la /tmp, cd>

//-------------------------------- SETTING UP ARGS FOR EACH COMMAND --------------------------------//

        // for each command, split into arguments
        // note: commands.size() returns "size_type", so typecast to int
        for (int i = 0; i < (int)commands.size(); i++)
        {
            char test_print[30] = "setting up args\n";
            write(STDERR_FILENO, test_print, strlen(test_print)); 

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
                exiting = true;
            }

            else if (split_args_vec[0] == "cd")
            {
                // 0 or more parameters
            }

            else if (split_args_vec[0] == "path")
            {
                // 0 or more parameters
            }

//-------------------------------- SHELL COMMANDS --------------------------------//

            else 
            {
                // get the path command that the user want to do
                string path = split_args_vec[0];
                string full_path = Path_checker(path);
                
                // if the path is valid, do stuff
                if (!full_path.empty())
                {
                    // run a fork to execv the command; because execv "replaces" current program running, doing it in a copy of this program allows us to keep running this "main" program
                    pid_t pid = fork();

                    if (pid == 0)
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

                        // char *const argv[] must be terminated by a NULL pointer
                        // use .data() so the vector can be treated as an array/pointers
                        execv(full_path.c_str(), argv_vec.data());

                        // if the execv above returns, that means an error happened, so print message then exit
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                        exit(1);
                    }

                     if (pid > 0) { wait(NULL); }
                }

                // if not valid directory, return error
                else
                {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                }

               
            }
        }
    }

    return 0;
}
