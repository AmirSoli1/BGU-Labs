#include <stdio.h>

int main(int argc, char* argv[]){
	int isDebug = 0;
	int sign = 0;
	char* key = "0";
	FILE* outstream = stdout;
	FILE* instream = stdin;
	for (int i = 0 ; i < argc ; i++){
		if (isDebug == 1 && (argv[i][0] != '-' || argv[i][1] != 'D' || argv[i][2] != '\0'))
			fprintf(stderr, "%s\n" ,argv[i]);
		switch (argv[i][0]){
			case '+':
				switch ((argv[i][1])){
					case 'D':
						isDebug = 1;
						break;
					case 'e':
						sign = 1;
						key = argv[i];
						break;
				}
				break;
			case '-':
				switch ((argv[i][1])){
					case 'D':
						isDebug = 0;
						break;
					case 'e':
						sign = -1;
						key = argv[i];
						break;
					case 'i':
						argv[i] += 2;
						instream = fopen(argv[i], "r");
						if (instream == NULL){
							fprintf(stderr, "no such file exists\n");
							return 1;
						}
						break;
					case 'o':
						argv[i] += 2;
						outstream = fopen(argv[i], "w");
						break;
				}
				break;
		}
	}
	int index = 2;
	while (1){
		char read;
		char write;
		read = fgetc(instream);
		int increment = (key[index] - 48) * sign;
		write = read + increment;
		if (read == EOF)
			break;
		else if (read >= 48 && read <= 57){
		 	if (write < 48)
				write += 10;
			if (write > 57)
				write -= 10;
			}
		else if (read >= 65 && read <= 90){
		 	if (write < 65)
				write += 26;
			if (write > 90)
				write -= 26;
			}
		else if (read >= 97 && read <= 122) {
		 	if (write < 97)
				write += 26;
			if (write > 122)
				write -= 26;
			}
		else
			write = read;
		fputc(write, outstream);
		index++;
		if (key[index] == '\0')
			index = 2;
	}
	fclose(instream);
	fclose(outstream);
}
