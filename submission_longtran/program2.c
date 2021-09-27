/**
 * A simple shell programs which can do the following:
 * + cd
 * + pwd
 * + mkdir
 * + echo
 * + exit
 * + call programs and executed using execv
 * + I/O redirection
 * + uncomplete piping.
 * 
 * Based on parser.c of the HINT code by professor Shaun Cooper
 * 
 * Authors: Long Tran
 * Date: Sept 26, 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CONSTANTS.h"
#include "utilities.h"

// global variables
char str_input[COMMAND_MAX_LENGTH + 1];
char * tokens[COMMAND_MAX_ARGUMENTS + 1];
int tkc;

// input stream path
// output stream path
char * input_stream_path;
char * output_stream_path;
int stdin_cp, stdout_cp;
// path name to be used by those who want to use the file path
char file_path_temp[FILE_PATH_MAX_SIZE+1];
char file_path_curr[FILE_PATH_MAX_SIZE+1];
char file_path_base[FILE_PATH_MAX_SIZE+1];

/**
 * Wrapper to use fprintf with the output stream of the shell
 */
int fprintf_wrapper(const char * s) {
    return fprintf(stdout, "%s", s);
} // end if

/**
 * Wrapper to use fgets with the input stream of the shell
 */
char * fgets_wrapper(char * buffer, int max_length) {
    return fgets(buffer, max_length, stdin);
} // end input

/*
    Parse the string for commands and arguments
    Modified from the parse function parsec.c HINT code by professor Cooper

    Return: Return the number of arguments scanned, -1 in error
*/
int parse(char *str, char *argv[]) {  
    int i,j;
    char *point;

    j= 0;
    // check bound
    if (j >= COMMAND_MAX_ARGUMENTS) {
        perror("The number of arguments passed the limit\n");
        return -1;
    } // end if

    // split the line into tokens
    point=strtok(str," ");

    // parse
    int must_be_fol = 0;
    while (point != NULL) {
        // assessing the syntax before storing
        if (must_be_fol) {
            if (strcmp(point, "<") == 0 || strcmp(point, ">") == 0) {
                fprintf(stderr, "Syntax error: Unexpected token after %s\n", point);
                return -1;
            } // end if
            must_be_fol = 0;
        } else if (strcmp(point, "<") == 0 || strcmp(point, ">") == 0) {
            must_be_fol = 1;
        } // end else if

        argv[j]=point;

        // continue to split the string
        point=strtok(NULL," ");

        // get to the next place to store argument
        j++;

        // check bound
        if (point != NULL && j >= COMMAND_MAX_ARGUMENTS) {
            perror("The number of arguments passed the limit\n");
            return -1;
        } // end if
    } // end while

    // end of string, check if there is any value that needs follow up, if there is 
    if (must_be_fol) {
        fprintf(stderr, "Syntax error: Unexpected token after %s\n", argv[j-1]);
        return -1;
    } // end if

    // end the string
    argv[j]=NULL;

    // return 
    return(j);
} // end parse

/*
    Wrapper function for getcwd to put the variable in argument path.
    Return: return what getcwd returns
*/
char * pwd(char * path) {
    return getcwd(path, FILE_PATH_MAX_SIZE+1);
} // end pwd

/*
    Execute one command section
    Return: The status EXIT_FAILURE or EXIT_SUCCESS or other exit codes
*/
int executeOneCommand(int t_start, int t_end, int * pipe) { 
    // arguments array that each arguments will use
    char * argv[COMMAND_MAX_ARGUMENTS];
    int argc = 0;

    // variable to scan the tokens
    int t_count = t_end - t_start;
    input_stream_path = NULL;
    output_stream_path = NULL;

    
    // get the input and output stream, choose the rightmost one
    int j;
    for (j = t_start; j < t_end;) {
        if (strcmp(tokens[j], ">") == 0) {
            output_stream_path = tokens[j+1];
            j = j + 2;
        } else if (strcmp(tokens[j], "<") == 0) {
            input_stream_path = tokens[j+1];
            j = j + 2;
        } else {
            argv[argc++] = tokens[j];
            j = j + 1;
        } // end if
    } // end for j
    argv[argc] = NULL;
    
    // if (t_start != 0) {
    //     char c;
    //     scanf("%c", &c);
    //     fprintf(stderr, "-- %c --", c);
    // } // end if
    

    // set the inputredirection
    int input_fd = -2, output_fd = -2;
    if (input_stream_path != NULL) {
        // try to open the file
        input_fd = open(input_stream_path, O_RDONLY);
        if (input_fd == -1) {
            if (errno == ENOENT) {
                perror("The redirect input file does not exist\n");
                return EXIT_FAILURE;
            } else {
                // unhandle error, just report it
                exit(errno);
            } // end else
        } // end if

        // redirect stdin to the file
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("Something is wrong when open the redirection input stream\n");
            return EXIT_FAILURE;
        } // end if
    }  // end if
    // set the output redirection
    if (output_stream_path != NULL) {
        // try to open the file
        output_fd = open(output_stream_path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (output_fd == -1) {
            // unhandled error
            exit(errno);
        } // end if

        // redirect stdout to the file
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("Something is wrong when open the redirection output stream\n");
            return EXIT_FAILURE;
        } // end if
    } // end if
    
    // if there is arguments, execute the program
    int status = EXIT_SUCCESS;
    if (argc > 0) {
        int start = 0, end = argc, count = end - start;
        // get the command to run
        char * command = argv[start];
        //fprintf(stderr, "%s\n", command);
        // check if it's valid
        // inate functions
        // + chdir: cd
        // + pwd: pwd
        // + echo: echo
        // + exit: exit
        if (strcmp(command, "cd") == 0) { // cd
            // chdir
            char * des_path = "";
            if (argc > 1) {
                des_path = argv[start+1];
            } else {
                // set the file path
                des_path = file_path_base;
            } // end else
            
            // change the dir
            int sts = chdir(des_path);

            // analyze error
            if (sts == -1) {
                switch (errno)  {
                    // Search permission is denied for one of the components of path.
                    case EACCES:
                        perror("cd: Search permission is denied for one of the components of path\n");
                        break;

                    // path points outside your accessible address space
                    case EFAULT:
                        perror("cd: path points outside your accessible address space\n");
                        break;
                    
                    // An I/O error occurred
                    case EIO:
                        perror("cd: An I/O error occurred\n");
                        break;
                    
                    // Too many symbolic links were encountered in resolving path
                    case ELOOP:
                        perror("cd: Too many symbolic links were encountered in resolving path\n");
                        break;

                    // path is too long
                    case ENAMETOOLONG: 
                        perror("cd: path is too long\n");
                        break;
                    
                    // The directory specified in path does not exist
                    case ENOENT: 
                        perror("cd: The directory specified in path does not exist\n");
                        break;

                    // Insufficient kernel memory was available.
                    case ENOMEM:
                        perror("cd: Insufficient kernel memory was available\n");
                        break;
                    
                    // A component of path is not a directory
                    case ENOTDIR:
                        perror("cd: A component of path is not a directory\n");
                        break;

                    // unknown error
                    default:
                        perror("cd: Unknown error\n");
                        break;
                } // end switch

                status = EXIT_FAILURE;
            } else { // change dir successfully
                // update the curernt dir string
                char * temp = pwd(file_path_temp);
                if (temp == NULL) {
                    switch (errno) {
                        // The size argument is 0.
                        case EINVAL:
                            perror("Failed to get current dir string: The size argument is 0\n");
                            break;
                        
                        // The size argument is greater than 0, but is smaller than the length of the pathname +1.
                        case ERANGE:
                            perror("Failed to get current dir string: The size argument is greater than 0, but is smaller than the length of the pathname +1\n");
                            break;

                        // Read or search permission was denied for a component of the pathname.
                        case EACCES:
                            perror("Failed to get current dir string: Read or search permission was denied for a component of the pathname\n");
                            break;

                        // Insufficient storage space is available.
                        case ENOMEM:
                            perror("Failed to get current dir string: Insufficient storage space is available\n");
                            break;

                        default: 
                            perror("Failed to get current dir string: Unknown error\n");
                            break;
                    } // end switch
                    exit(EXIT_FAILURE);
                } // end if
                // assign to the current directory
                strcpy(file_path_curr, file_path_temp);
            } // end else
            
        } else if (strcmp(command, "pwd") == 0) { // pwd
            // pwd
            if (pwd(file_path_temp) == NULL) {
                switch (errno) {
                    // The size argument is 0.
                    case EINVAL:
                        perror("dev error: The size argument is 0\n");
                        break;
                    
                    // The size argument is greater than 0, but is smaller than the length of the pathname +1.
                    case ERANGE:
                        perror("dev error: The size argument is greater than 0, but is smaller than the length of the pathname +1\n");
                        break;

                    // Read or search permission was denied for a component of the pathname.
                    case EACCES:
                        perror("pwd: Read or search permission was denied for a component of the pathname\n");
                        break;

                    // Insufficient storage space is available.
                    case ENOMEM:
                        perror("pwd: Insufficient storage space is available\n");
                        break;

                    default: 
                        perror("pwd: Unknown error\n");
                        break;
                } // end switch
                status = EXIT_FAILURE;
            } else {
                fprintf_wrapper(file_path_temp);
                fprintf_wrapper("\n");
            } // end if
        } else if (strcmp(command, "echo") == 0) { // echo
            // echo the arguments with one space afteward except the last one
            int i;
            for (i = start+1; i < end-1; ++i) {
                fprintf_wrapper(argv[i]);
                fprintf_wrapper(" ");
            } // end for i

            // echo the last argument if there is at least one argument (not counting the command)
            if (count > 1)
                fprintf_wrapper(argv[end-1]);

            // new line
            fprintf_wrapper("\n");
        } else if (strcmp(command, "exit") == 0) { // exit
            if (count > 2) {
                perror("Exit Error: Too many arguments\n");
                status = EXIT_FAILURE;
            } else {
                // find the status
                if (count > 1) {
                    status = atoi(argv[1]);
                } // end if 

                // exit
                perror("exiting...\n");
                exit(status);
            } // end else
        } else if (strcmp(command, "mkdir") == 0) { // mkdir
            // get the path
            char * des_path = "";
            if (argc > 1) {
                int dir_idx;
                for (dir_idx = 0; dir_idx < argc-1; ++dir_idx) {
                    des_path = argv[start + 1 + dir_idx];
                    int sts = mkdir(des_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH);
                    // analyze error
                    if (sts == -1) {
                        switch(errno) {
                            // Search permission is denied on a component of the path prefix, or write permission is denied on the parent directory of the directory to be created.
                            case EACCES:
                                perror("mkdir: Search permission denied\n");
                                break;
                            
                            // The named file exists.
                            case EEXIST:
                                fprintf(stderr, "mkdir: %s already existed\n", des_path);
                                break;

                            // A loop exists in symbolic links encountered during resolution of the path argument.
                            case ELOOP:
                                perror("mkdir: A loop exists in symbolic links encountered during resolution of the path argument\n");
                                break;

                            // The link count of the parent directory would exceed {LINK_MAX}.
                            case EMLINK:
                                perror("mkdir: \n");
                                break;

                            // The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.
                            // or As a result of encountering a symbolic link in resolution of the path argument, the length of the substituted pathname string exceeded {PATH_MAX}.
                            case ENAMETOOLONG:
                                perror("mkdir: The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}\
                                or As a result of encountering a symbolic link in resolution of the path argument, the length of the substituted pathname string exceeded {PATH_MAX}\n");
                                break;
                            
                            // A component of the path prefix specified by path does not name an existing directory or path is an empty string.
                            case ENOENT:
                                perror("mkdir: A component of the path prefix specified by path does not name an existing directory or path is an empty string\n");
                                break;
                            
                            // The file system does not contain enough space to hold the contents of the new directory or to extend the parent directory of the new directory.
                            case ENOSPC:
                                perror("mkdir: The file system does not contain enough space to hold the contents of the new directory or to extend the parent directory of the new directory\n");
                                break;

                            // A component of the path prefix is not a directory.
                            case ENOTDIR:
                                perror("mkdir: A component of the path prefix is not a directory\n");
                                break;

                            // The parent directory resides on a read-only file system.
                            case EROFS:
                                perror("mkdir: The parent directory resides on a read-only file system\n");
                                break;
                            
                            // unknown error
                            default: 
                                perror("mkdir: Unknown error\n");
                                break;
                        } // end switch
                    
                        status = EXIT_FAILURE;
                    } // end if
                } // end for dir_i
            } else {
                perror("mkdir: no arguments inputed\n");
                status = EXIT_FAILURE;
            } // end else
            
        } else { // others
            // others  
            // create a child process to run the program
            int ID = fork();  
            if (ID == 0) { // children
                int status = EXIT_SUCCESS;
                status = execv(argv[0], argv);
                if (DEBUG)
                    perror("Exiting from child\n");

                if (status == -1) {
                    switch (errno) {
                        //The argument list and the environment is larger than the system limit of ARG_MAX bytes.
                        case E2BIG: 
                            perror("The argument list and the environment is larger than the system limit of ARG_MAX bytes\n");
                            break;
                        //The calling process doesn't have permission to search a directory listed in path, or it doesn't have permission to execute path, or path's filesystem was mounted with the ST_NOEXEC flag.
                        case EACCES:
                            perror("The calling process doesn't have permission to search a directory listed in path, or it doesn't have permission to execute path, or path's filesystem was mounted with the ST_NOEXEC flag\n");
                            break;
                        
                        // Either argv or the value in argv[0] is NULL. 
                        case EINVAL:
                            perror("Either argv or the value in argv[0] is NULL\n");
                            break;

                        // Too many levels of symbolic links or prefixes. 
                        case ELOOP:
                            perror("Too many levels of symbolic links or prefixes\n");
                            break;

                        //  Insufficient resources available to load the new executable image or to remap file descriptors.
                        case EMFILE:
                            perror("Insufficient resources available to load the new executable image or to remap file descriptors\n");
                            break;

                        // The length of path or an element of the PATH environment variable exceeds PATH_MAX.
                        case ENAMETOOLONG:
                            perror("The length of path or an element of the PATH environment variable exceeds PATH_MAX\n");
                            break;

                        // One or more components of the pathname don't exist, or the path argument points to an empty string.
                        case ENOENT:
                            perror("One or more components of the pathname don't exist, or the path argument points to an empty string\n");
                            break;

                        // The new process's image file has the correct access permissions, but isn't in the proper format. 
                        case ENOEXEC:
                            perror("The new process's image file has the correct access permissions, but isn't in the proper format\n");
                            break;
                        // There's insufficient memory available to create the new process.
                        case ENOMEM:
                            perror("There's insufficient memory available to create the new process\n");
                            break;

                        // A component of path isn't a directory.
                        case ENOTDIR:
                            perror("A component of path isn't a directory\n");
                            break;

                        // The text file that you're trying to execute is busy (e.g. it might be open for writing). 
                        case ETXTBSY:
                            perror("The text file that you're trying to execute is busy (e.g. it might be open for writing)\n");
                            break;
                        
                        default:
                            perror("Unknown error\n");
                            break;
                    } // end switch

                    // set status to failure
                    status = EXIT_FAILURE;
                } // end if

                // close all the pipes. 
                if (pipe != NULL) {
                    close(pipe[0]);
                    close(pipe[1]);
                } // end if

                // release resources. 
                if (input_fd != -2) {
                    if (close(input_fd) == -1) {
                        perror("Cannot close the redirected stream\n");
                        // ignore the error
                        exit(EXIT_FAILURE);
                    } else {
                        if (DEBUG) 
                            perror("close input parent successfully\n");
                    } // end else
                }  // end if
                // release resources
                if (output_fd != -2) {
                    if (close(output_fd) == -1) {
                        perror("Cannot close the redirected output stream\n");
                        // ignore the error
                        exit(EXIT_FAILURE);
                    } else {
                        if (DEBUG) 
                            perror("close output parent successfully\n");
                    } // end else 
                } // end if

                // exit from child process with this status
                exit(status);
            } else if (ID < 0) { // error
                perror("Cannot create a child to execute the process\n");
                status = EXIT_FAILURE;
            } else { // parent
                // get the status of the child
                if (DEBUG)
                    perror("waiting for the child process\n");

                // need to test to see if the error will actually prints out
                pid_t c_id;
                // wait for a program to finish
                for (c_id = wait(&status);!WIFEXITED(status);c_id = wait(&status));
                //perror("child id: %d status: %d\n", c_id, WEXITSTATUS(status));
                if (DEBUG)
                    perror("the child process ended\n");
                    
                // return success status
                status = WEXITSTATUS(status);
            } // end else
        } // end else
    } // end if
    
    // redirect back to the original
    if (input_fd != -2) {
        if (close(input_fd) == -1) {
            perror("Cannot close the redirected stream\n");
            // ignore the error
            return -1;
        } else {
            // redirect to the original standard in
            if (dup2(stdin_cp, STDIN_FILENO) == -1) {
                perror("Something is wrong when redirecting the stdout back to the original\n");
                exit(EXIT_FAILURE);
            } // end if
            if (DEBUG) 
                perror("reset parent input successfully\n");
            if (DEBUG) 
                perror("close input parent successfully\n");
        } // end else
        
    }  // end if
    // redirect back the to the original
    if (output_fd != -2) {
        if (close(output_fd) == -1) {
            perror("Cannot close the redirected output stream\n");
            // ignore the error
            return -1;
        } else {
            // redirect to the standard out
            if (dup2(stdout_cp, STDOUT_FILENO) == -1) {
                perror("Something is wrong when redirecting the stdin back to the original\n");
                exit(EXIT_FAILURE);
            } // end if

            if (DEBUG) 
                perror("reset parent output successfully\n");
            if (DEBUG) 
                perror("close output parent successfully\n");
        } // end else 
        
    } // end if

    // the program was executed successfully
    return status;
} // end executeOneCommand

/*
    Execute the command line
    Return: EXIT_SUCESS or EXIT_FAILURE or other codes
*/
int execute() {
    int status = EXIT_SUCCESS;

    // count the number of tokens
    int pipe_count = 0;
    int i;    
    for (i = 0; i < tkc; ++i) {
        if (strcmp(tokens[i], "|") == 0) 
            pipe_count++;
    } // end for i

    int ** pipe_fd = NULL;
    if (pipe_count > 0) {
        pipe_fd = malloc(sizeof(int *) * pipe_count);
        if (pipe_fd == NULL) {
            perror("Cannot allocate spaces for pipes\n");
            status = EXIT_FAILURE;
            return status;
        } // end if

        for (i = 0; i < pipe_count; ++i) {
            pipe_fd[i] = malloc(sizeof(int) * 2);
            if (pipe_fd[i] == NULL) {
                perror("Cannot allocate spaces for pipes\n");
                status = EXIT_FAILURE;
                break;
            } // end if
            
            pipe_fd[i][0] = -1;
            pipe_fd[i][1] = -1;

            if (pipe(pipe_fd[i]) == -1) {
                perror("Cannot create pipe to connect two commands\n");
                status = EXIT_FAILURE;
                break;
            } // end if
        }  // end for i
    } // end if
    
    // boolean values
    int hasPipeAfter = 0;
    int hasPipeBefore = 0;
    int pipe_index = 0;
    // index
    int end, start = 0;
    // analyze and execute
    for (end = 0; end <= tkc && status != EXIT_FAILURE; ++end) {
        // if it's the end of the tokens list, no pipe after
        if (end == tkc) {
            hasPipeAfter = 0;
        } else if (strcmp(tokens[end], "|") == 0) { // has pipe after
            hasPipeAfter = 1;
        } // end else if

        // if we have pipe we will fork to do the process
        if (hasPipeAfter || hasPipeBefore) {
            // create child process
            int ID = fork();
            if (ID < 0) { // error
                perror("Cannot create a child process to execute the commands in the pipe\n");
                status = EXIT_FAILURE;
                break;
            } else if (ID == 0) { // child process
                int * pipe;
                if (hasPipeAfter) {
                    close(pipe_fd[pipe_index][0]);
                    pipe = pipe_fd[pipe_index];
                    if (dup2(pipe_fd[pipe_index][1], STDOUT_FILENO) == -1) {
                        perror("Cannot redirect the output pipe");
                        fprintf(stderr, "index: %d - pipe_fd: %d - errno: %d\n", pipe_index, pipe_fd[pipe_index][1], errno);
                        exit(EXIT_FAILURE);
                    } // end if
                } // end if
                
                // has 
                if (hasPipeBefore) {
                    close(pipe_fd[pipe_index-1][1]);
                    pipe = pipe_fd[pipe_index-1];
                    if (dup2(pipe_fd[pipe_index-1][0], STDIN_FILENO) == -1) {
                        perror("Cannot redirect the input pipe");
                        status = EXIT_FAILURE;
                        exit(EXIT_FAILURE);
                    } // end if
                } // end if 
        
                status = executeOneCommand(start, end, pipe);

                // close the pipe
                if (hasPipeAfter) {
                    fflush(stdout);
                    int status = close(pipe_fd[pipe_index][1]);
                } // end if

                if (hasPipeBefore) {
                    close(pipe_fd[pipe_index-1][0]);
                } // end if

                exit(status);
            } else { // parent process
                // close pipe before
                if (hasPipeBefore) {
                    close(pipe_fd[pipe_index-1][0]);
                    hasPipeBefore = 0;
                } // end if

                if (hasPipeAfter) {
                    close(pipe_fd[pipe_index][1]);
                    start = end + 1;
                    hasPipeBefore = 1;
                    hasPipeAfter = 0;
                    pipe_index++;
                } // end if
            } // end else
        } else if (end == tkc) { // no pipe execute one process
            status = executeOneCommand(start, end, NULL);
        } // end else
    } // end for end

    //
    if (pipe_count) {
        int status;
        pid_t c_id;
        // wait for children processes to finish
        while ((c_id = wait(&status)) > 0);
    } // end if

    //close all the pipes
    if (pipe_fd != NULL) {
        for (i = 0; i < pipe_count; ++i) {
            close(pipe_fd[i][0]);
            close(pipe_fd[i][1]);
            free(pipe_fd[i]);
        } // end for i

        free(pipe_fd);
    } // end if
    
    return status;
} // end execute


/*
    Main program to start the shell
*/
int main(void) {
    // setup
    // no input stream path at the start.
    input_stream_path = NULL;
    output_stream_path = NULL;

    // save the initial stdin and stdout files
    stdin_cp = dup(STDIN_FILENO);
    stdout_cp = dup(STDOUT_FILENO);

    // get the base dir
    char * temp = pwd(file_path_base);
    if (temp == NULL) {
        switch (errno) {
            // The size argument is 0.
            case EINVAL:
                perror("Failed to get current dir for setup: The size argument is 0\n");
                break;
            
            // The size argument is greater than 0, but is smaller than the length of the pathname +1.
            case ERANGE:
                perror("Failed to get current dir for setup: The size argument is greater than 0, but is smaller than the length of the pathname +1\n");
                break;

            // Read or search permission was denied for a component of the pathname.
            case EACCES:
                perror("Failed to get current dir for setup: Read or search permission was denied for a component of the pathname\n");
                break;

            // Insufficient storage space is available.
            case ENOMEM:
                perror("Failed to get current dir for setup: Insufficient storage space is available\n");
                break;

            default: 
                perror("Failed to get current dir for setup: Unknown error\n");
                break;
        } // end switch
        exit(EXIT_FAILURE);
    } // end if
    // assign to the current directory
    strcpy(file_path_curr, file_path_base);

    // infinite loop
    while (1) {
        // prompt to enter command
        printf("%s%% ", file_path_curr);
        fflush(stdout);
        
        // read in command
        if (fgets(str_input, COMMAND_MAX_LENGTH+1, stdin) == NULL) {
            perror("Error: Cannot read the command\n");
            sleep(2);
            continue;
        } // end if

        // trim the string
        int length = trim(str_input);
        if (length == 0) {
            //perror("Empty command\n");
            continue;
        } // end if

        // parse the string for the arguments
        tkc = parse(str_input, tokens);
        // check for parse error
        if (tkc == -1) {
            continue;
        } // end if

        // int i;
        // for (i = 0; i < argc; ++i) {
        //     perror("arg1: %s\n", tokens[i]);
        // } // end if

        // try to execute the command
        int status = execute();
    } // end while
} // end main