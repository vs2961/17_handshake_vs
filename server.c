#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>


static void sighandler(int signo) {
    if (signo == SIGINT) {
        unlink("./public");
        unlink("./sender");
        unlink("./reciever");
        exit(0);
    }
}


void setup_handshake() {
    mkfifo("./public", 0664);
    printf("Pipe created. Waiting for client...\n");
    int pub_pipe;
    pub_pipe = open("./public", O_RDONLY);

    char client_pid[256];
    int status = read(pub_pipe, client_pid, sizeof(client_pid));
    if (status == -1) {
        printf("Error reading from client. %s\n", strerror(errno));
    }
    unlink("./public");
    printf("Connecting with client %s...\n", client_pid + 2);
    char recieved[] = "You have been acknowledged!\n";
    int secret_pipe = open(client_pid, O_WRONLY);
    status = write(secret_pipe, recieved, sizeof(recieved));
    if (status == -1) {
        printf("Error connecting to client pipe. %s\n", strerror(errno));
    }
    status = read(pub_pipe, client_pid, sizeof(client_pid)); 
    if (status == -1) {
        printf("Unable to read from client.\n");
    }
    if (strcmp(client_pid, "connected") != 0) {
        printf("Unable to finalize connection.\n");
    }
    printf("You have been connected!\n");
}


int main()
{
    signal(SIGINT, sighandler);
    while (1) {
        setup_handshake();
        char * info = "./sender";
        char * processed = "./reciever";
        
        mkfifo(info, 0664);
        mkfifo(processed, 0664);

        int inf = open(info, O_RDONLY);
        int proc = open(processed, O_WRONLY);
        
        char buffer[256];
        while (1) {
            read(inf, buffer, 256);
            if (strcmp(buffer, "exit\n") == 0) {
                break;
            }
            char *token;
            char *p = buffer;
            int total = 0;
            while (p) {
                token = strsep(&p, " ");
                total += atoi(token);
            }
            sprintf(buffer, "%d", total);
            write(proc, buffer, sizeof(buffer)); 
        }
        close(inf);
        close(proc);
        unlink(info);
        unlink(processed);
        printf("Client Disconnected.\n\n");
    }
}
