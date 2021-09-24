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
char * argv[COMMAND_MAX_ARGUMENTS];
int argc;
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
        return -1;
    } // end if

    // split the line into tokens
    point=strtok(str," ");

    // parse
    while (point != NULL) {
        // assing the arguments into the token
        argv[j]=point;

        // continue to split the string
        point=strtok(NULL," ");

        // get to the next place to store argument
        j++;

        // check bound
        if (point != NULL && j >= COMMAND_MAX_ARGUMENTS) {
            return -1;
        } // end if
    } // end while

    // end the string
    argv[j]=NULL;

    // return 
    return(j);
} // end parse

/*
    Try to execute the program
*/
int execute(int argc, char *argv[]) {
    input_stream = NULL;
    output_stream = NULL;
    // get the input and output stream
    int j;
    for (j = argc - 1; j >= 0; ++j) {
        if (strcmp(argv[j], ">")) {
            if (argv[j+1] == NULL) {
                printf("Expect something after >\n");
                return EXIT_FAILURE;
            } // end if

            output_stream = argv[j+1];
        } else if (strcmp(argv[j], "<")) {
            if (argv[j+1] == NULL) {
                printf("Expect something after <\n");
                return EXIT_FAILURE;
            } // end if

            input_stream = argv[j+1];
        } // end else if
    } // end for j
    
    // get the command to run
    char * command = argv[0];

    // check if it's valid
    // inate functions
    // + chdir: cd
    // + pwd: pwd
    // + echo: echo
    // + exit: exit
    if (strcmp(command, "cd") == 0) {
        // chdir
        printf("chdir\n");
    } else if (strcmp(command, "pwd") == 0) {
        // pwd
        printf("pwd\n");
    } else if (strcmp(command, "echo") == 0) {
        // echo the arguments with one space afteward except the last one
        int i;
        for (i = 1; i < argc-1; ++i) {
            fprintf_wrapper(argv[i]);
            fprintf_wrapper(" ");
        } // end for i

        // echo the last argument if there is at least one argument (not counting the command)
        if (argc > 1)
            fprintf_wrapper(argv[argc-1]);

        // new line
        fprintf_wrapper("\n");
    } else if (strcmp(command, "exit") == 0) {
        if (argc > 2) {
            printf("Exit Error: Too many arguments\n");
            return EXIT_FAILURE;
        } // end if

        // find the status
        int status = 0;
        if (argc > 1) {
            status = atoi(argv[1]);
        } // end if 

        // exit
        printf("exiting...\n");
        exit(status);
    } else {
        // others  
        // create a child process to run the program
        int ID = fork();  
        if (ID == 0) {
            int status = EXIT_SUCCESS;
            status = execv(argv[0], argv);
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

        // get the stastus of the child
        // need to test to see if the error will actually prints out
        int status = EXIT_SUCCESS;
        pid_t c_id;
        // wait for a program to finish
        for (c_id = wait(&status);!WIFEXITED(status);c_id = wait(&status));
        //printf("child id: %d status: %d\n", c_id, WEXITSTATUS(status));

        // return success status
        return WEXITSTATUS(status);
    } // end else

    // the program was executed successfully
    return EXIT_SUCCESS;
} // end run

/*
    Main program to start the shell
*/
int main(void) {
    // setup
    input_stream = stdin;
    output_stream = stdout;

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
        argc = parse(str_input, argv, &input_stream, &output_stream);
        // check for parse error
        if (argc == -1) {
            printf("The number of arguments passed the limit\n");
            continue;
        } // end if

        // int i;
        // for (i = 0; i < argc; ++i) {
        //     printf("arg1: %s\n", argv[i]);
        // } // end if

        // try to execute the command
        int status = execute(argc, argv);
    } // end while
} // end main