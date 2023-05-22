#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
	printf("Select operation from the following menu:\n");
	char* input = malloc(100);
	while(fgets(input, 100, stdin) != NULL){
		printf("Select operation from the following menu:\n");
	//	fgets(input, 100, stdin);
	//	if(sizeof(input) > 0 && input[0] == EOF)
	//		break;
	}
}
