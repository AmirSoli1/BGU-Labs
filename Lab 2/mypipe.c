#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){
    pid_t pid;
    int pipefd[2];
    char message[6] = "hello\n";
    char readingMessage[sizeof(message)];

    // making pipe and handle pipe error 
    if (pipe(pipefd) == -1) {
        fprintf(stderr,"pipe() error");
        exit(1);
    }

    // child
    if(!(pid=fork())){
        if (write(pipefd[1],message,sizeof(message)) == -1) {
            // write() error
            fprintf(stderr,"write() error");
            exit(1);
        }
        //close write end of the pipe after sending message
        close(pipefd[1]);
        exit(0);
    }

    // fork() error
    else if (pid == -1) {
        fprintf(stderr,"fork() error");
        exit(1);
    }

    // parent
    else {  
        if (read(pipefd[0],readingMessage,sizeof(readingMessage)) == -1) {
            // read() error
            fprintf(stderr,"read() error");
            exit(1);
        }
        printf("incoming message: ");
        printf("%s", readingMessage);

        // close read end of the pipe after sending message
        close(pipefd[0]);  
        exit(0);
    }

    return 0;
}