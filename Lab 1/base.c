#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  for (int i = 0; i < array_length ; i++){
	mapped_array[i] = (*f)(array[i]);  
  }
  return mapped_array;
}

char my_get(char c){
	return fgetc(stdin);
}
/* Ignores c, reads and returns a character from stdin using fgetc. */

char cprt(char c){
	if (c >= 0x20 && c <= 0x7E)
		printf("%c\n", c);
	else
		printf(".\n");
	return c;
}
/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */

char encrypt(char c){
	if (c >= 0x20 && c <= 0x7E)
		return c+1;
	return c;
}
/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */

char decrypt(char c){
	if (c >= 0x20 && c <= 0x7E)
		return c-1;
	return c;
}
/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */

char xprt(char c){
	if (c >= 0x20 && c <= 0x7E)
		printf("%x\n",c);
	else
		printf(".\n");
	return c;
}
/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */

 
struct fun_desc {
char *name;
char (*fun)(char);
};


int main(int argc, char **argv){
	char* carray = malloc(5);
	char* input = malloc(3);
	carray[0] = '\0';
	struct fun_desc menu[] = { { "Get string", my_get }, { "Print string", cprt }, { "Encrypt", encrypt } , { "Decrypt", decrypt} ,{ "Print Hex", xprt} , { NULL, NULL }};
	
	while(1){
		printf("Select operation from the following menu:\n");
		int i = 0;
		while(menu[i].name){
			printf("%d)  %s\n", i , menu[i].name);
			i++;
		}
		printf("Option : ");
		if(!fgets(input, 3, stdin)){
			free(carray);
			free(input);
			break;
		}
		if(input[0]){
		
			if(input[0] >= '0' && input[0] <= '5'){
				printf("\nWithin bounds\n");
				char* tmp;
				tmp = map(carray, 5, menu[input[0] - '0'].fun);
				free(carray);
				carray = tmp;
				printf("DONE.\n\n");
			}
			else {
				printf("Not within bounds\n");
				break;
			}
		}
		
	}
}












