#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"

/*
    Trim the value of the string in-place. Remove leading and trailing whitespaces/newlines
    Return: -1 if the string is null, else the length of the trimmed string.
*/
int trim(char * str) {
    if (str == NULL)
        return -1;
    
    int i;
    int start = -1, end;

    // find the start with no leading whitespaces
    for (i = 0; str[i] != '\0'; ++i) {
        if (start == -1 && str[i] != ' ' && str[i] != '\n') {
            start = i;
        } // end if
    } // end if

    // if start is not set to an index
    // we know that the string has only whitespaces
    if (start == -1) {
        return 0;
    } // end if

    // get the end where the NULL is
    end = i;
    // find the end with no trailing whitespaces
    for (i = end-1; str[i] == ' ' || str[i] == '\n'; --i);
    // assign the true end
    end = i;

    // get the length of the trimmed string
    int length = end - start + 1;

    // repositioning the characters
    for (i = 0; i < length; ++i) {
        str[i] = str[start + i];
    } // end for i

    // put an NULL character to end the string
    str[length] = '\0';

    //printf("|%s|\n", str);

    // return the length of the trimmed string
    return length;
} // end trim