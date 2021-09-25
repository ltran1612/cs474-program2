#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
	perror("program2");
	char s[100];
	gets(s);
	printf("%s\n", s);
//	exit(EXIT_FAILURE);
} // end main
