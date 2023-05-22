#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <signal.h>
#include <string.h>

int debug =0;
char* history[20];
int newest = 0;
int oldest = 0;
int size = 0;

void add_to_history(char* cmd) {
    if (cmd[0] !='!'){
        if (size == 20) {  
            free(history[oldest]);  
            oldest = (oldest + 1) % 20; 
            size--;
        }
        free(history[newest]);
        history[newest] = strdup(cmd);  //  make copy 
        newest = (newest + 1) % 20; 
        size++;
    }
}

void printHistory() {
    int i;
    for (i = oldest; i < oldest + size; i++) {
        printf("%d: ", i - oldest + 1);  
        printf("%s", history[i % 20]);  
    }
}

typedef struct process{
        cmdLine* cmd; /* the parsed command line*/
        pid_t pid;/* the process id that is running the command*/
        int status;/* status of the process: RUNNING/SUSPENDED/TERMINATED */
        struct process *next;/* next process in chain */
    } process;

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* newProcess = (process*)malloc(sizeof(process)); 
    newProcess->cmd = cmd;
    newProcess->pid = pid;
    newProcess->status = 1;
    newProcess->next = *process_list; 
    *process_list = newProcess;   
}
void printProcessList(process** process_list) {
    printf("Pid   Command    Status\n"); 
    updateProcessList(process_list);
    process* current = process_list[0];
    process* prev = NULL;

     while (current != NULL) { 
        printf("%d   ", current->pid);
        printf("%s   ", current->cmd->arguments[0]);
        if (current->status == 1){printf("Running   \n");}
        else if(current->status == -1){
            printf("Terminated   \n");
            if (prev == NULL) {
                process_list[0] = current->next;  
            } else {
                prev->next = current->next;
            }
            process* temp = current;
            current = current->next;
            free(temp);
            continue;
        }
        else {printf("Suspended   \n");}
        prev = current;
        current = current->next; 
        
     } 
} 

void freeProcessList(process* process_list){
    process* current = process_list;
    while (current != NULL) { 
        process* temp = current;
        current = current->next;
        free(temp);
    }
}
void freeHistory(){
    int i =0;
    while (i < size){
       if (history[(oldest + i) % 20] != NULL){
            free(history[(oldest + i) % 20]);
            
        }
       i++;
    }
}
void updateProcessList(process **process_list){
    process* current = *process_list;
    while (current != NULL) { 
        int waitResult = waitpid(current->pid,&current->status,WNOHANG); 
        if (waitResult == current->pid){
            current->status = -1;
        }
        current = current->next;
    }
}

 void updateProcessStatus(process** process_list, int pid, int status){
    process* current = *process_list;
    while (current != NULL) { 
        if(current->pid  == pid){
            current->status = status;
            break;
        }
        current = current->next;
    }
 }
void debugPrint(cmdLine *pCmdLine){
    fprintf(stderr, "Executing command: ");
    fprintf(stderr,"%s\n", pCmdLine->arguments[0]);
    fprintf(stderr,"%s","PID: ");
    fprintf(stderr,"%d\n", getpid());
}
int execute(cmdLine *pCmdLine,process** process_list){
    pid_t pid;
    int status;
    FILE* inFile = NULL;
    FILE* outFile = NULL;


    //the shell commands
        if (strcmp(pCmdLine->arguments[0],"cd") == 0){
            if (chdir(pCmdLine->arguments[1]) == -1) {
                if (debug){debugPrint(pCmdLine);}
                fprintf(stderr,"cd: ");
                fprintf(stderr,"%s: ",pCmdLine->arguments[1]);
                fprintf(stderr,"No such file or directory\n");
            }
            freeCmdLines(pCmdLine);
            return 0;
        }
        // sig section
        char *sigToRead = pCmdLine->arguments[0];
        if(strcmp(sigToRead,"suspend") == 0){
            pid_t PidToSig = atoi(pCmdLine->arguments[1]);
            if (kill(PidToSig, SIGTSTP) == -1) {
                if (debug){debugPrint(pCmdLine);}
                fprintf(stderr,"suspend error\n");
            }
            else{
                updateProcessStatus(process_list,PidToSig,0);
                printf("suspend worked\n");
            }
            freeCmdLines(pCmdLine);
            return 0;
         }
         if(strcmp(sigToRead,"wake") == 0){
            pid_t PidToSig = atoi(pCmdLine->arguments[1]);
            if (kill(PidToSig, SIGCONT) == -1) {
                if (debug){debugPrint(pCmdLine);}
                fprintf(stderr,"wake error\n"); 
            }
            else{
                updateProcessStatus(process_list,PidToSig,1);
                printf("wake worked\n");
            }
            freeCmdLines(pCmdLine);
            return 0;
         }
          if(strcmp(sigToRead,"kill") == 0){
            pid_t PidToSig = atoi(pCmdLine->arguments[1]);  
            if (kill(PidToSig, SIGINT) == -1) {
                if (debug){debugPrint(pCmdLine);}
                fprintf(stderr,"kill error\n"); 
            }
            else{
                updateProcessStatus(process_list,PidToSig,-1);
                printf("kill worked\n"); 
            }
            freeCmdLines(pCmdLine);
            return 0;
         }
        if(strcmp(sigToRead,"history") == 0){
            printHistory();
            freeCmdLines(pCmdLine);
            return 0;
        }
         if(strcmp(sigToRead,"procs") == 0){
            printProcessList(process_list);
            return 0;
        }
        if (strcmp(pCmdLine->arguments[0],"quit") == 0){
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(0);
        }
        if(strcmp(sigToRead,"!!") == 0){
            if (size == 0) {
                fprintf(stderr, "No commands in history\n");
                printProcessList;
                return 0;
            } 
            cmdLine *last_cmd = parseCmdLines(history[(newest-1)%20]); // parse last command
            add_to_history(history[(newest-1)%20]); // add last command to history
            execute(last_cmd, process_list); // execute last command
            freeCmdLines(pCmdLine);
            return 0;
        }
        if((sigToRead[0] == '!') && (atoi(sigToRead +1) != 33)){
            if ((atoi(sigToRead +1) < 0) || (atoi(sigToRead +1) > 20)){
                fprintf(stderr,"error in range\n");
                freeCmdLines(pCmdLine);
                return 0;
            } 
            int x = atoi(sigToRead +1);
            cmdLine *command = parseCmdLines(history[x-1]); // parse last command
             if (history[x-1] == NULL){
                fprintf(stderr,"error: commad does not exist");
                freeCmdLines(pCmdLine);
                return 0;
            }
            add_to_history(history[x-1]); // add command to history
            execute(command, process_list); // execute  command
            freeCmdLines(pCmdLine);
            return 0;
        }

    //the line parser found the pipe
    if(pCmdLine->next != NULL){
        pid_t pid1, pid2;
        int pipefd[2];
        // input output redirection error
        if (pCmdLine->next->inputRedirect != NULL) {
            if (debug){debugPrint(pCmdLine);}
            fprintf(stderr, "error redirect input of right side\n");
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1);
        }
        if (pCmdLine->outputRedirect != NULL) {
            if (debug){debugPrint(pCmdLine);}
            fprintf(stderr, "error redirect input of left side\n");
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1);
        }
    
        if(pipe(pipefd) == -1){
            if (debug){debugPrint(pCmdLine);}
            fprintf(stderr,"pipe() error");
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1);
        }
        // first child
        if(!(pid1=fork())){
            //input
            if (pCmdLine->inputRedirect != NULL){
                inFile = fopen(pCmdLine->inputRedirect, "r+");
                if (inFile == NULL){
                    if (debug){debugPrint(pCmdLine);}
                    fprintf(stderr, "input redirection error\n");
                    freeCmdLines(pCmdLine);
                    freeProcessList(*process_list);
                    freeHistory();
                    exit(1);
                }
                dup2(fileno(inFile), 0);
            }
            
            close(STDOUT_FILENO);// Close the standard output
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);// close write end the was duplicted
            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
                if (debug){debugPrint(pCmdLine);}
                fprintf(stderr,"execvp() error");
                if (inFile != NULL){fclose(inFile);}
                if (outFile != NULL){fclose(outFile);}
                freeCmdLines(pCmdLine);
                freeProcessList(*process_list);
                freeHistory();
                exit(1);
            }
        }
        // first child fork() error
        else if (pid1 == -1) {
            if (debug){debugPrint(pCmdLine);}  
            fprintf(stderr,"fork() error");
            if (inFile != NULL){fclose(inFile);}
            if (outFile != NULL){fclose(outFile);}
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1);
        }

        close(pipefd[1]);// close write end of the pipe

        // second child
        if(!(pid2=fork())){
            // output
            if (pCmdLine->next->outputRedirect != NULL){
                outFile = fopen(pCmdLine->next->outputRedirect, "w+");
                if (outFile == NULL){
                    if (debug){debugPrint(pCmdLine);}
                    fprintf(stderr, "output redirection error\n");
                    freeCmdLines(pCmdLine);
                    freeProcessList(*process_list);
                    freeHistory();
                    exit(1);
                }
                dup2(fileno(outFile), 1);
            }
            close(STDIN_FILENO);// Close the standard input.
            dup2(pipefd[0],STDIN_FILENO); 
            close(pipefd[0]);// close read end the was duplicted
    
            if (execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments) == -1) {
                if (debug){debugPrint(pCmdLine);}
                fprintf(stderr,"execvp() error");
                if (inFile != NULL){fclose(inFile);}
                if (outFile != NULL){fclose(outFile);}
                freeCmdLines(pCmdLine);
                freeProcessList(*process_list);
                freeHistory();
                exit(1);
            }
        }
        // second fork() error
        else if (pid2 == -1) {
            if (debug){debugPrint(pCmdLine);}
            fprintf(stderr,"fork() error");
            if (inFile != NULL){fclose(inFile);}
            if (outFile != NULL){fclose(outFile);}
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1);
        }
        
        close(pipefd[0]); // close read end of the pipe

        // wait for the childs to complete
        int status1;
        addProcess(process_list,pCmdLine,pid1);   
        if (waitpid(pid1, &status1, 0) == -1) {
            if (debug){debugPrint(pCmdLine);}
            fprintf(stderr,"waitpid()) error");
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1);
        }  
        int status2;
        addProcess(process_list,pCmdLine,pid2); 
        if (waitpid(pid2, &status2, 0) == -1) {
            if (debug){debugPrint(pCmdLine);}
            fprintf(stderr,"waitpid()) error");
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1);
        }
        
    }

    /// no pipe
    else{
        // child
        if(!(pid=fork())){
            // input
            if (pCmdLine->inputRedirect != NULL){
                inFile = fopen(pCmdLine->inputRedirect, "r+");
                if (inFile == NULL){
                    if (debug){debugPrint(pCmdLine);}
                    fprintf(stderr, "input redirection error\n");
                    freeCmdLines(pCmdLine);
                    freeProcessList(*process_list);
                    freeHistory();
                    exit(1);
                }
                dup2(fileno(inFile), 0);
            }
            // output
            if (pCmdLine->outputRedirect != NULL){
                outFile = fopen(pCmdLine->outputRedirect, "w+");
                if (outFile == NULL){
                    if (debug){debugPrint(pCmdLine);}
                    fprintf(stderr, "output redirection error\n");
                    freeCmdLines(pCmdLine);
                    freeProcessList(*process_list);
                    freeHistory();
                    exit(1);
                }
                dup2(fileno(outFile), 1);
            }
            if ((execvp(pCmdLine->arguments[0],pCmdLine->arguments)) == -1){
                if (debug){debugPrint(pCmdLine);}
                fprintf(stderr, "error in execvp()\n");
                if (inFile != NULL){fclose(inFile);}
                if (outFile != NULL){fclose(outFile);}
                freeCmdLines(pCmdLine);
                freeProcessList(*process_list);
                freeHistory();
                exit(1);
            }
            
        }
        // fork error
        else if (pid == -1){
            if (debug) {debugPrint(pCmdLine);}
            fprintf(stderr, "error in fork()\n"); 
            if (inFile != NULL){fclose(inFile);}
            if (outFile != NULL){fclose(outFile);}
            freeCmdLines(pCmdLine);
            freeProcessList(*process_list);
            freeHistory();
            exit(1); 
        }

        // parent 
        else{
            addProcess(process_list,pCmdLine,pid); 
            if (pCmdLine->blocking) {waitpid(pid, &status, 0);}
            if (WIFEXITED(status)) {updateProcessStatus(process_list,pid,-1);}
            if (WIFSIGNALED(status)) {updateProcessStatus(process_list,pid,1);}
            if (status && debug) {debugPrint(pCmdLine);}
            if (inFile != NULL){fclose(inFile);}
            if (outFile != NULL){fclose(outFile);} 
            
        }
    } 
    freeCmdLines(pCmdLine);   
    return 1;
}

int main(int argc, char **argv){
    char shell[PATH_MAX];
    process* process_list = NULL;  

    while(1){
        if (getcwd(shell, sizeof(shell)) != NULL) {
            printf("%s$ ",shell);
        } 
        else {
            fprintf(stderr,"getcwd() error");
            freeProcessList(process_list);
            freeHistory();
            exit(1);
        }
        char in[2048];// reading to the shell
        if (fgets(in, 2048, stdin) == NULL) {
            break; 
        }
        if (in[1] == '\0'){
            continue;
        }
        cmdLine *pCmdLine = parseCmdLines(in);// parse
        add_to_history(in);
        
        // debug mode
        int i=0;
        while (argv[i] != NULL){
            if (strcmp(argv[i],"-d") == 0){
                debug = 1;
            }
            i++;  
        }
        execute(pCmdLine,&process_list);       
    }
    return 0;
}