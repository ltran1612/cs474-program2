/**
 * The starting point of the shell
 * 
 * Based on parser.c of the HINT code by professor Shaun Cooper
 * 
 * Authors: Long Tran
 * Date:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "CONSTANTS.h"
#include "utilities.h"

// global variables
char str_input[COMMAND_MAX_LENGTH];
char * tokens[COMMAND_MAX_ARGUMENTS];
int tkc;
char * input_stream;
char * output_stream;

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
        printf("The number of arguments passed the limit\n");
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
                printf("Syntax error: Unexpected token after %s\n", point);
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
            printf("The number of arguments passed the limit\n");
            return -1;
        } // end if
    } // end while

    // end of string, check if there is any value that needs follow up, if there is 
    if (must_be_fol) {
        printf("Syntax error: Unexpected token after %s\n", argv[j-1]);
        return -1;
    } // end if

    // end the string
    argv[j]=NULL;

    // return 
    return(j);
} // end parse

/*
    Try to execute the program
*/
int execute(int tkc, char *tokens[]) {
    char * argv[COMMAND_MAX_ARGUMENTS];
    int argc = 0;
    input_stream = NULL;
    output_stream = NULL;

    int t_start = 0, t_end = tkc, t_count = t_end - t_start;
    

    // get the input and output stream
    int j;
    for (j = t_start; j < t_end;) {
        if (strcmp(tokens[j], ">") == 0) {
            output_stream = tokens[j+1];
            j = j + 2;
        } else if (strcmp(tokens[j], "<") == 0) {
            input_stream = tokens[j+1];
            j = j + 2;
        } else {
            argv[argc++] = tokens[j];
            j = j + 1;
        } // end if
    } // end for j
    argv[argc] = NULL;
    
    int start = 0, end = argc, count = end - start;

    // do thing in a child process
    int f_id = fork();
    if (f_id == 0) { // child process
        // set the standard in and out
        if (output_stream != NULL) {
            FILE * out_stream = freopen(output_stream, "w", stdout);
            if (out_stream == NULL) {
                exit(EXIT_FAILURE);
            } // end if
        } // end if
       
        if (input_stream != NULL) {
            FILE* in_stream = freopen(input_stream, "r", stdin);
             if (input_stream == NULL) {
                exit(EXIT_FAILURE);
            } // end if
        } // end if
        
        // get the command to run
        char * command = argv[start];

        // check if it's valid
        // inate functions
        // + chdir: cd
        // + pwd: pwd
        // + echo: echo
        // + exit: exit
        if (strcmp(command, "cd") == 0) { // cd
            // chdir
            printf("chdir\n");
        } else if (strcmp(command, "pwd") == 0) { // pwd
            // pwd
            printf("pwd\n");
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
                printf("Exit Error: Too many arguments\n");
                return EXIT_FAILURE;
            } // end if

            // find the status
            int status = 0;
            if (count > 1) {
                status = atoi(argv[1]);
            } // end if 

            // exit
            printf("exiting...\n");
            exit(status);
        } else { // others 
            int status = EXIT_SUCCESS;
            status = execv(command, argv);
            if (status == -1) {
                switch (errno) {
                    //The argument list and the environment is larger than the system limit of ARG_MAX bytes.
                    case E2BIG: 
                        printf("The argument list and the environment is larger than the system limit of ARG_MAX bytes\n");
                        break;
                    //The calling process doesn't have permission to search a directory listed in path, or it doesn't have permission to execute path, or path's filesystem was mounted with the ST_NOEXEC flag.
                    case EACCES:
                        printf("The calling process doesn't have permission to search a directory listed in path, or it doesn't have permission to execute path, or path's filesystem was mounted with the ST_NOEXEC flag\n");
                        break;
                    
                    // Either argv or the value in argv[0] is NULL. 
                    case EINVAL:
                        printf("Either argv or the value in argv[0] is NULL\n");
                        break;

                    // Too many levels of symbolic links or prefixes. 
                    case ELOOP:
                        printf("Too many levels of symbolic links or prefixes\n");
                        break;

                    //  Insufficient resources available to load the new executable image or to remap file descriptors.
                    case EMFILE:
                        printf("Insufficient resources available to load the new executable image or to remap file descriptors\n");
                        break;

                    // The length of path or an element of the PATH environment variable exceeds PATH_MAX.
                    case ENAMETOOLONG:
                        printf("The length of path or an element of the PATH environment variable exceeds PATH_MAX\n");
                        break;

                    
                    // One or more components of the pathname don't exist, or the path argument points to an empty string.
                    case ENOENT:
                        printf("One or more components of the pathname don't exist, or the path argument points to an empty string\n");
                        break;

                    // The new process's image file has the correct access permissions, but isn't in the proper format. 
                    case ENOEXEC:
                        printf("The new process's image file has the correct access permissions, but isn't in the proper format\n");
                        break;
                    // There's insufficient memory available to create the new process.
                    case ENOMEM:
                        printf("There's insufficient memory available to create the new process\n");
                        break;

                    // A component of path isn't a directory.
                    case ENOTDIR:
                        printf("A component of path isn't a directory\n");
                        break;

                    // The text file that you're trying to execute is busy (e.g. it might be open for writing). 
                    case ETXTBSY:
                        printf("The text file that you're trying to execute is busy (e.g. it might be open for writing)\n");
                        break;
                } // end switch
            } // end if

            // exit from child process with this status
            exit(status);
        } // end if

    } else { // parent process
        // get the stastus of the child
        // need to test to see if the error will actually prints out
        int status = EXIT_SUCCESS;
        pid_t c_id;
        // wait for a program to finish
        for (c_id = wait(&status);!WIFEXITED(status);c_id = wait(&status));
        printf("child id: %d status: %d\n", c_id, WEXITSTATUS(status));

        // return success status
        return WEXITSTATUS(status);
    } // end else

    // the program was executed successfully
    exit(EXIT_SUCCESS);
} // end run

/*
    Main program to start the shell
*/
int main(void) {
    // setup
    input_stream = NULL;
    output_stream = NULL;

    // infinite loop
    while (1) {
        // prompt to enter command
        printf("%% ");
        fflush(stdout);
        
        // read in command
        if (fgets(str_input, COMMAND_MAX_LENGTH, stdin) == NULL) {
            printf("Error: Cannot read the command\n");
            continue;
        } // end if

        // trim the string
        int length = trim(str_input);
        if (length == 0) {
            //printf("Empty command\n");
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
        //     printf("arg1: %s\n", tokens[i]);
        // } // end if

        // try to execute the command
        int status = execute(tkc, tokens);
    } // end while
} // end main