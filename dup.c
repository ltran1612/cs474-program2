
//DUP.C




/* example of running an exec family program with STDIN set to a different file descripter */
 

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/errno.h>
 

main()
{

    int fp;
    pid_t child;
    int status;

    fp=open("/etc/passwd",O_RDONLY);

    child=0;
 
 child=fork();
 

   if ( child != 0 )
    {printf("I am a parent waiting\n");
     waitpid(child, &status, 0);
     printf("waiting complete\n");
    }

  /* reset the STDIN file descriptor before going to the execl program */

  else {  if ( dup2(fp,STDIN_FILENO) == EBADF) {printf("bad dup\n");}

          if (execl("/bin/cat","/bin/cat","-",NULL) != 0)
            { printf("Cannot execute \n");
              exit(1);
            }
       }

}
  
