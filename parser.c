
PARSER.C

/* simple program which takes input from the keyboard and parses it into an argv vector */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>

/* function which takes a character buffer and parses the buffer into tokens using th
    strtok subroutine */

/*returns the number of arguments read in  */

int parse(char* buff, char* argv[])
{  int i,j;
   char *point;

   j=-1;

   point=strtok(buff,"  ");

   while (point != NULL)
     {

        j++;
        argv[j]=point;
        point=strtok(NULL,"     ");
     }
     argv[j+1]='\0';

     return(j);
}
 

main()
{
    int args;
    int i;

    char line[200];
    char* argv[100];

while (1)
 {
   /* Print the prompt */
   printf("%% ");
   fflush(stdout);

   /* read in the line */
   gets(line);

   /*parse the line -- note that the *WHILE* line is in the argv[] vector */

   args=parse(line,argv);
 

   printf("The following was read in and parsed\n");
    for (i=0;i<=args;i++)
      printf("%s\n",argv[i]);
   printf("end of input\n");

   printf("\n");
 }
}
  
