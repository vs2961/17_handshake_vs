#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

int wantExit = 0;

static void sighandler(int signo) {
    if (signo == SIGINT) {
        printf("\nExited! Press enter to exit.\n");
        wantExit = 1;
    }
}

void connect_server() {
    printf("Connecting to server...\n");
    int pub_pipe;
    pub_pipe = open("./public", O_WRONLY);
    if (pub_pipe == -1) {
        printf("Server is offline.\n");
        exit(0);
    }
    int status; 
    char toSend[256];
    sprintf(toSend, "./%d", getpid());
    mkfifo(toSend, 0664);
    status = write(pub_pipe, toSend, sizeof(toSend));
    if (status == -1) {
        printf("Error writing to server pipe. %s\n", strerror(errno));
    }
    printf("Waiting for response from server...\n");
    int priv_pipe = open(toSend, O_RDONLY);
    if (priv_pipe == -1) {
        printf("Unable to open private pipe");
    }

    char recieved[256];
    status = read(priv_pipe, recieved, sizeof(recieved));
    if (status == -1) {
        printf("Error reading from private pipe. %s\n", strerror(errno));
    }
    if (strcmp("You have been acknowledged!\n", recieved) != 0) {
        printf("Unable to connect to the server.\n");
    }
    else {
        printf("You have been connected!\n");
        unlink(toSend);
    }
}

int main() {
    signal(SIGINT, sighandler);
    connect_server();

    char * sender = "./sender";
    char * reciever = "./reciever";
    
    int fd1 = open(sender, O_WRONLY);
    int fd2 = open(reciever, O_RDONLY);

    char buffer[256];
    
    printf("\nThis program adds numbers. Enter numbers selected by a space to get their sum.\n");
    printf("Type \"exit\" to exit.\n");
    while(1) {
        if (wantExit) {
            break;
        }
        printf("\nEnter numbers to be added: ");
        fgets(buffer, 256, stdin);
        if (strcmp(buffer, "exit\n") == 0 || wantExit) {
            write(fd1, "exit\n", 10);
            break;
        }
        write(fd1, buffer, sizeof(buffer));
        read(fd2, buffer, sizeof(buffer));
        printf("Your sum is: %s\n", buffer);
    }

    close(fd1);
    close(fd2);
    unlink(sender);
    unlink(reciever);

    return 0;
}
