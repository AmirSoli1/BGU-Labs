#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){
    pid_t pid1, pid2;
    int pipefd[2];

    // making pipe and handle pipe error 
    if (pipe(pipefd) == -1) {
        fprintf(stderr,"pipe() error");
        exit(1);
    }
    fprintf(stderr, "(parent_process>forking...)\n");

    // first child
    if(!(pid1=fork())){
        close(STDOUT_FILENO);
        if (dup(pipefd[1]) == -1) {
            fprintf(stderr,"dup() error");
            exit(1);
    }
    fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        close(pipefd[1]);
        fprintf(stderr, "child1>going to execute cmd: lsss)\n");
        execlp("ls", "ls", "-l", NULL);
    }
    // first fork() error
    else if (pid1 == -1) {
        fprintf(stderr,"fork() error");
        exit(1);
    }
    fprintf(stderr, "(parent_process>created process with id: %d)\n", pid1);

    fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
    // close write end
    close(pipefd[1]);

    // second child
    if(!(pid2=fork())){
        close(STDIN_FILENO);
        if (dup(pipefd[0]) == -1) {
            fprintf(stderr,"dup() error");
            exit(1);
        }
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
        close(pipefd[0]);
        fprintf(stderr, "child2>going to execute cmd: tail....)\n");
        execlp("tail", "tail", "-n", "2", NULL);
    }
    // second fork() error
    else if (pid2 == -1) {
        fprintf(stderr,"fork() error");
        exit(1);
    }

    fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");    
    // close read end
    close(pipefd[0]);

    fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
    // wait for the childs  to complete
    int status1;
    if (waitpid(pid1, &status1, 0) == -1) {
        fprintf(stderr,"waitpid()) error");
        exit(1);
    }  
    int status2;
    if (waitpid(pid2, &status2, 0) == -1) {
        fprintf(stderr,"waitpid()) error");
        exit(1);
    }

    fprintf(stderr, "(parent_process>exiting...)\n");
    return 0;
}