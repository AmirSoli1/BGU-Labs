#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct virus {
unsigned short SigSize;
char virusName[16];
unsigned char* sig;
} virus; 

typedef struct link link;
struct link {
link *nextVirus ;
virus *vir;
};

struct fun_desc {
  char *name;
  link* (*fun)(link*);
};

void printHex(FILE* input, int length){
    unsigned * buf = malloc(length);
    fread(buf,length,1,input);
    unsigned int  x = buf[0];
    for (int i = 0; i < length; i++){
       printf("%02X ", buf[i]); 
    }

}

virus* readVirus(FILE* input){
    
    unsigned char sigLength[2];
    char myName[16];
    fread(sigLength, 2, 1, input);
    if(feof(input)) return 0;
    fread(myName, 16, 1, input);
    unsigned short mySigLength = sigLength[0] + sigLength[1] * 256;   

    unsigned char* mySignature = malloc(mySigLength);
    fread(mySignature, mySigLength, 1, input);

    struct virus* myVirus = malloc(sizeof(virus));
    (*myVirus).SigSize = mySigLength;
    for(int i = 0; i < 16 ; i++)
        (*myVirus).virusName[i] = myName[i];
    (*myVirus).sig = mySignature;
    return myVirus;
}

void printVirus(virus* virus, FILE* output){
    
    fprintf(output, "%s" , "Virus name: ");
    fprintf(output, "%s", (*virus).virusName);
    fputs("\n", output);
    
    fprintf(output, "%s" , "Virus size: ");
    fprintf(output, "%d", (*virus).SigSize);
    fputs("\n", output);
    
    fprintf(output, "%s" , "signature: \n");
    for(int i = 0; i < (*virus).SigSize; i++){
        if (i % 20 == 0 && i != 0)
            fprintf(output, "\n");
        fprintf(output, "%02X ", (*virus).sig[i]);
    }
    fputs("\n\n", output);
}

void list_print(link *virus_list, FILE* output){
    while(virus_list){
        printVirus((*virus_list).vir, output);
        virus_list = (*virus_list).nextVirus;
    }
}

link* list_append(link* virus_list, virus* data){
    link* newLink = malloc(16);
    (*newLink).vir = data;
    (*newLink).nextVirus = NULL;
    if(virus_list)
        (*newLink).nextVirus = virus_list;
    return newLink;
}

void list_free(link *virus_list){
    while(virus_list){
        virus* data = (*virus_list).vir;
        free((*data).sig);
        free(data);
        link* linkToFree = virus_list;
        virus_list = (*virus_list).nextVirus;
        free(linkToFree);
    }
}

link* load_signatures(link *virus_list){
    printf("Enter file name: \n");
    char fileName[100];
    scanf("%s",fileName);
    FILE* input = fopen(fileName, "r");
    if (input == NULL){
            printf("\nFile does not exist\n");
            return NULL;
        }
    char magic[4];
    fread(magic, 1, 4, input);
    magic[4] = '\0';
    if(strcmp(magic, "VISL") != 0){
        printf("\nIncorrect magic number, please load a different file\n");
    }
    else{
        while(!feof(input)){
            virus* nextVirus = readVirus(input);
            if (nextVirus)
                virus_list = list_append(virus_list, nextVirus);
        }
    }
    fclose(input);
    return virus_list;
}

void detect_virus(char *buffer, unsigned int size, link *virus_test){
    int counter = 0;
    while(virus_test){
        char *startPosition = buffer;
        virus *curr = virus_test->vir;
        unsigned short siglength = curr->SigSize;
        unsigned char* mySignature = curr->sig;

        for(char *c = startPosition; c != buffer + size - siglength + 1 ; c++){
            if (memcmp(c,mySignature, siglength) == 0){
                counter++;
                printf("\nstarting byte location: %ld\n", c-buffer);
                printf("virus name: %s\n", curr->virusName);
                printf("signature size: %d\n", siglength);
            }
        }
        virus_test = virus_test->nextVirus;
    }
    if(counter == 0)
        printf("\nNo viruses detected, the file is clean!\n");
    else
        printf("\nTotal number of viruses detected: %i\n", counter);
}

link* print_signatures(link *virus_list){
    list_print(virus_list, stdout);
    return virus_list;
}

FILE* infectedFile = 0;

link* detect_viruses(link *virus_list){
    if(infectedFile){
        char buffer[10000];
        fseek(infectedFile, 0, SEEK_END);
        long fileSize = ftell(infectedFile);
        fseek(infectedFile, 0, SEEK_SET);
        fread(buffer, 1, fileSize, infectedFile);
        detect_virus(buffer, fileSize, virus_list);
    }
    else
        printf("\nProgram did not recieve a file as argument\n");
    return virus_list;
}
char *file = 0;

void neutralize_virus(char *fileName, int signatureOffset){
    char clean = 0xC3;
    FILE* openFile = fopen(fileName, "r+b");
    fseek(openFile, signatureOffset, SEEK_SET);
    fwrite(&clean, 1 ,1 ,openFile);
    fclose(openFile);
}

void fix_viruses(char *buffer, unsigned int size, link *virus_test){
    int counter = 0;
    while(virus_test){
        char *startPosition = buffer;
        virus *curr = virus_test->vir;
        unsigned short siglength = curr->SigSize;
        unsigned char* mySignature = curr->sig;

        for(char *c = startPosition; c != buffer + size - siglength + 1 ; c++){
            if (memcmp(c,mySignature, siglength) == 0){
                counter++;
                neutralize_virus(file, c - buffer);
            }
        }
        virus_test = virus_test->nextVirus;
    }
    if(counter == 0)
        printf("\nNo viruses detected, the file is clean!\n");
    else
        printf("\nTotal number of viruses eliminated: %i\n", counter);
}



link* fix_file(link *virus_list){
    if(infectedFile){
        char buffer[10000];
        fseek(infectedFile, 0, SEEK_END);
        long fileSize = ftell(infectedFile);
        fseek(infectedFile, 0, SEEK_SET);
        fread(buffer, 1, fileSize, infectedFile);
        fix_viruses(buffer, fileSize, virus_list);
    }
    else
        printf("\nProgram did not recieve a file as argument\n");
    return virus_list;
}

link* quit_program(link *virus_list){
    list_free(virus_list);
    if(infectedFile)
        fclose(infectedFile);
    printf("Program closed\n");
    return NULL;
}


int main(int argc, char **argv){
    file = argv[1];
    infectedFile = fopen(file, "r");
	char input[100];
	link* virusList = 0;
	struct fun_desc menu[] = { { "Load signatures", load_signatures }, { "Print signatures", print_signatures }, { "Detect viruses", detect_viruses } , { "Fix file", fix_file} ,{ "Quit", quit_program} , { NULL, NULL }};
	int bound = 0;
	while(menu[bound].name)
	      bound++;
	while(1){
		printf("\nSelect operation from the following menu:\n");	
		int i = 0;
		while(menu[i].name){
		    printf("%d)  %s\n", i+1 , menu[i].name);
		    i++;
		}
		printf("Option : ");
		scanf("%s", input);
		if(input == NULL){
		    quit_program(virusList);
		    break;
		}
		int value = atoi(input);
		if(value >= 1 && value <= bound){
            if(value == 1){
                list_free(virusList);
                virusList = 0;
            }
		    virusList = menu[value - 1].fun(virusList);
		    if(value == 5)
		        break;
		}
		else {
		    printf("Number not within bounds, program closing\n");
            quit_program(virusList);
		    break;
		}
	}
    return 0;
}

