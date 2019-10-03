//import libraries
#include <vector>
#include <iostream>
#include <unistd.h>
#include "handler.h"
#include <signal.h>
#include <typeinfo>
#include <fcntl.h>
#include <sys/wait.h>

//set namespace
using namespace std;

//declare / initialize variables
char cwd[256];
vector<string> path = {"/bin/","/usr/bin/"};

//function to handle pwd
void pwd(){

    //get cwd using getwd system call
    getcwd(cwd, sizeof(cwd));

    //output results
    cout << cwd;
    cout << "\n";
}

//function to handle cd
void change_directory(char const *path){

    //use chdir system call to change directory
    int dir = chdir(path);

    //return error if necessary
    if (dir < 0) {
        cout << "No such file or directory\n";
    }
}

//function to handle exiting shell
void exit_program(){

    //print message
    cout << "Exiting\n";

}

//function to display path
void show_path() {

    //print message
    cout << "Current PATH: ";

    //iterate through directories in PATH
    for (string dir : path) {
        //display path
        cout << dir;

        //if not last element append colon
        if (dir != path.back()){
            cout << ":";
        }
    }

    //display newline
    cout << "\n";
}

//function to execute command
void execute(const char* cmd, vector<string> input) {

    //declare / initialize variables
    char *argv[input.size()+1];
    char *env[] = {NULL};
    int i = 0;
    int pid;

    //append command and arguments to array of character pointers
    for (string test : input) {
        argv[i] = (char *)test.c_str();
        i++;
    }

    //terminatate array with null
    argv[i] = NULL;

    //if error in fork return error
    if ((pid = fork()) == -1) {
        perror("fork error");
    }

    //enter child process
    else if (pid == 0) {
        //return error if necessary
        if(execve(cmd, argv, env) == -1){
            perror("execve");
        }
        _exit(0);

    }

    //if parent wait for child
    else {
        wait(NULL);
    }

}


//function to handle external program execution
void external_execution(vector<string> input) {
    //declare and initialize things
    const char* cmd_1 = input[0].c_str();
    const char* cmd_2;
    int succ = 0;

    //check if commands exists in cwd, if so execute
    if (access(cmd_1, F_OK) == 0){
        succ = 1;
        execute(cmd_1, input);
    }

    //check if commands exists in path, if so execute
    for (string wow: path) {
        wow += cmd_1;
        cmd_2 = wow.c_str();

        if (access(cmd_2, F_OK) == 0){
            succ = 1;
            execute(cmd_2, input);
            break;
        }

    }

    //report error if command doesnt exist
    if (!succ) {
        cout << "Error: Program not found";
    }
}

//append directories to path
void append_path(string item) {
    path.push_back(item);

}

//function to handle output redirection
void redirect_output(vector<string> output, string location) {
    //declare and init variables
    int pid;
    const char * loc = (char*)location.c_str();

    //report fork error if necessary
    if ((pid = fork()) == -1) {
        perror("fork error");
    }

    //enter child process
    else if (pid == 0) {
        //open file
        int file_desc = open(loc, O_CREAT | O_WRONLY, 0666);

        //report error if necessary
        if(file_desc < 0) {
            cout << "Error opening the file" << "\n";
        }

        //create alias for stdou with file, and execute command
        dup2(file_desc, 1);
        external_execution(output);
        close(file_desc);
        _exit(0);

    }

    //if parent wait for child
    else {
        wait(NULL);
    }

}

void execute_pipe(vector<string> cmd1, vector<string> cmd2) {
    //declare and inititialize variables
    int p[2];
    int p1,p2;
    int i = 0;
    char *argv1[cmd1.size()+1];
    char *argv2[cmd2.size()+1];
    char *env[] = {NULL};


    for (string cmd : cmd1) {
        argv1[i] = (char *)cmd.c_str();
        i++;
    }

    argv1[i] = NULL;
    i = 0;



    for (string cmd : cmd2) {
        argv2[i] = (char *)cmd.c_str();
        i++;
    }

    argv2[i] = NULL;


    if (pipe(p) < 0) {
        cout << "error";
    }
    cout << "hi";

    p1 = fork();


    if (p1 < 0) {
        cout << "fork error";
    }




    //first child process is running
    if (p1 == 0) {
        //write process to pipe
        close(p[0]);
        dup2(p[1], 1);
        close(p[1]);


        if (execve(argv1[0], argv1, env) < 0) {
            //before this should probable check if file exists
            cout << "couldn't execute process 1";
        }
    }


    //parent process
    else {
        //create second child
        p2 = fork();

        if (p2 < 1) {
            cout << "fork error";
        }

        //child process is running
        if (p2 == 0){
            //only need to acess read end
            close(p[1]);
            dup2(p[0],1);
            close(p[1]);


            if (execve(argv2[0], argv2,env) < 0) {
                 cout << "couldn't execute process 2";
            }

        } else {
            //parrent is executing
            //wait for two children
            wait(NULL);

            wait(NULL);

        }

    }
}

void handle_signal(int signum) {
    cout << "wow";
}
