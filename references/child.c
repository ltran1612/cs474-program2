
//CHILD.C

 


/* This program gives a simple example of a parent process spawning
   a child process.  The parent waits on the child.

   If you change the child sleep time to larger values, you can
   use "ps -augx | grep  <userid>" on a different terminal to see
   your two processes: the parent waiting on the child

   HINT: This is one of the main components in the shell programming
   assignment.

   HINT: execl (or one of its varients) will be useful in executing
         user commands.
*/
/*
  Child.c

  Downloaded from the HINT code of the assignment.
  Modified by: Long Tran
*/

#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

main()
{

    int status;
    pid_t child;

   printf("From the main process\n");

   child=fork();

   /* the parent portion */
   if ( child != 0 )
    {printf("I am a parent waiting\n");
     waitpid(child, &status, 0);
     printf("waiting complete\n");
     exit(0);
    }

  /* the child portion */
  else { printf("I am a child executing a new environment \n");
         execl("/bin/ls","ls","/",(char *)0);
         /* at this point any other code is not run by the child */
         exit(0);}

}
  
