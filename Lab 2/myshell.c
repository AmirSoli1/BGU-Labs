#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <signal.h>
#include <string.h>


int debug =0;

void debugPrint(cmdLine *pCmdLine){
    fprintf(stderr, "Executing command: ");
    fprintf(stderr,"%s\n", pCmdLine->arguments[0]);
    fprintf(stderr,"%s","PID: ");
    fprintf(stderr,"%d\n", getpid());
}

int execute(cmdLine *pCmdLine){
    pid_t pid;
    int status;
    FILE* inFile = NULL;
    FILE* outFile = NULL;

    // child
    if(!(pid=fork())){
        // input and output redirection
        // input
        if (pCmdLine->inputRedirect != NULL){
            inFile = fopen(pCmdLine->inputRedirect, "r+");
            if (inFile == NULL){
                if (debug)
                    debugPrint(pCmdLine);
                fprintf(stderr, "input redirection error\n");
                freeCmdLines(pCmdLine);
                exit(1);
            }
            dup2(fileno(inFile), 0);
        }
        // output
        if (pCmdLine->outputRedirect != NULL){
            outFile = fopen(pCmdLine->outputRedirect, "w+");
            if (outFile == NULL){
                if (debug)
                    debugPrint(pCmdLine);
                fprintf(stderr, "output redirection error\n");
                freeCmdLines(pCmdLine);
                exit(1);
            }
            dup2(fileno(outFile), 1);
        }
    
        if ((execvp(pCmdLine->arguments[0],pCmdLine->arguments)) == -1){
            if (debug)
                debugPrint(pCmdLine);
            fprintf(stderr, "error in execvp()\n");

            if (inFile != NULL){
                fclose(inFile);
            }
            if (outFile != NULL){
                fclose(outFile);
            }
            freeCmdLines(pCmdLine);
            exit(1);
        }

        
    }
    // fork error
    else if (pid == -1){
        if (debug)
            debugPrint(pCmdLine);
        fprintf(stderr, "error in fork()\n"); 

        if (inFile != NULL){
            fclose(inFile);
        }
        if (outFile != NULL){
            fclose(outFile);
        }
        freeCmdLines(pCmdLine);
        exit(1); 
    }

    // parent 
    else{
        if (pCmdLine->blocking) {
            waitpid(pid, &status, 0);
        }

        if (status && debug)
            debugPrint(pCmdLine);

        if (inFile != NULL){
            fclose(inFile);
        }
        if (outFile != NULL){
            fclose(outFile);
        }    
          
    }
    
    return 1;
}
int main(int argc, char **argv){
    char shell[PATH_MAX];

    while(1){
        if (getcwd(shell, sizeof(shell)) != NULL) {
            printf("%s$ ",shell);
        } 
        else {
            fprintf(stderr,"getcwd() error");
            exit(1);
        }
        
        // reading to the shell
        char in[2048];
        if (fgets(in, 2048, stdin) == NULL) {
            break; 
        }
        if (in[1] == '\0'){
            continue;
        }
        cmdLine *pCmdLine = parseCmdLines(in);// parse
        
        // debug mode
        int i=0;
        while (argv[i] != NULL){
            if (strcmp(argv[i],"-d") == 0){
                
                debug = 1;
            }
            i++;  
        }
        // cd call
        if (strcmp(pCmdLine->arguments[0],"cd") == 0){
            if (chdir(pCmdLine->arguments[1]) == -1) {
                if (debug)
                    debugPrint(pCmdLine);
                fprintf(stderr,"cd: ");
                fprintf(stderr,"%s: ",pCmdLine->arguments[1]);
                fprintf(stderr,"No such file or directory\n");
            }
            freeCmdLines(pCmdLine);
            continue;
        }

        // sig section
        char *sigToRead = pCmdLine->arguments[0];
        if(strcmp(sigToRead,"suspend") == 0){
            pid_t PidToSig = atoi(pCmdLine->arguments[1]);
            if (kill(PidToSig, SIGTSTP) == -1) {
                if (debug)
                    debugPrint(pCmdLine);
                fprintf(stderr,"suspend error\n"); 
            }
            else{
                printf("suspend worked\n");
            }
            freeCmdLines(pCmdLine);
            continue;
        }
         if(strcmp(sigToRead,"wake") == 0){
            pid_t PidToSig = atoi(pCmdLine->arguments[1]);
            if (kill(PidToSig, SIGCONT) == -1) {
                if (debug)
                    debugPrint(pCmdLine);
                fprintf(stderr,"wake error\n"); 
            }
            else{
                printf("wake worked\n");
            }
            freeCmdLines(pCmdLine);
            continue;
        }
          if(strcmp(sigToRead,"kill") == 0){
            pid_t PidToSig = atoi(pCmdLine->arguments[1]);  
            if (kill(PidToSig, SIGINT) == -1) {
                if (debug)
                    debugPrint(pCmdLine);
                fprintf(stderr,"kill error\n"); 

            }
            else{
                printf("kill worked\n"); 
            }
            freeCmdLines(pCmdLine);
            continue;
        }
        
        // quit from the shell
        if (strcmp(pCmdLine->arguments[0],"quit") == 0){
            freeCmdLines(pCmdLine);
            exit(0);

        }
          
        execute(pCmdLine);        
        freeCmdLines(pCmdLine);
    }
    return 0;

}