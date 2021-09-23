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
#include "CONSTANTS.h"
#include "utilities.h"

char str_input[COMMAND_MAX_LENGTH];
char * argv[COMMAND_MAX_ARGUMENTS];
int argc;

void output(char * s) {

} // end if

void input(char * s) {

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
        // echo
        int i;
        for (i = 1; i < argc - 1; ++i) {
            //printf("%s", );
        } // end for i

        //printf("\n");
    } else if (strcmp(command, "exit") == 0) {
        // find the status
        int status = 0;
        // exit
        //printf("exiting with status...\n");
        exit(status);
    } else {
        // others
        printf("others\n");
    } // end else

    // the program was executed successfully
    return 0;
} // end run

/*
    Main program to start the shell
*/
int main(void) {
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
        argc = parse(str_input, argv);
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