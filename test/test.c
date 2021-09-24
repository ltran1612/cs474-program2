#include <stdio.h>
int main(int argc, char * argv[]) {
	printf("argc count: %d\n", argc);
	char s[100];
	gets(s);
	printf("%s\n", s);
} // end main
