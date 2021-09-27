/*
    Header file for functions that will be used in the program
*/
#ifndef UTILITIES_H
#define UTILITIES_H
/*
    Trim the value of the string in-place. Remove leading and trailing whitespaces/newlines
    Return: -1 if the string is null, else the length of the trimmed string.
*/
int trim(char * str);
#endif