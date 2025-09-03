// ||Link of the pcap file: https://drive.google.com/drive/folders/1BT9e0kvHX6i-VzwbnAu_gIltqW5uKpJR?usp=drive_link            ||

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);
    servaddr.sin_addr.s_addr = inet_addr("127.127.127.127");

    int n;
    socklen_t len;
    len = sizeof(servaddr);
    char requested_file[MAXLINE];
    printf("Enter name of the file you want to request: ");
    fgets(requested_file, MAXLINE, stdin);
    // Remove newline character if present
    requested_file[strcspn(requested_file, "\n")] = 0;

    sendto(sockfd, (const char *)requested_file, strlen(requested_file), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    printf("File request sent to server...\n");

    char buffer[MAXLINE];
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
    buffer[n] = '\0';

    if (strncmp(buffer, "NOTFOUND", 8) == 0) {
        printf("File %s Not Found\n", requested_file);
    } else {
        if (strcmp(buffer, "HELLO\n") == 0) {
            char newFile[MAXLINE];
            strcpy(newFile,requested_file);
            strcat(newFile,"(copy)");
            FILE *fptr = fopen(newFile, "w");
            if (fptr == NULL) {
                printf("Error opening file!\n");
                close(sockfd);
                exit(1);
            }
            fprintf(fptr, "%s", buffer);
            char *request = malloc(MAXLINE + 1);
            int word = 1;
            do {
                sprintf(request, "WORD%d", word);
                printf("Sending request (length %ld): %s\n", strlen(request), request);
                sendto(sockfd, (const char *)request, strlen(request), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
                memset(buffer, 0, sizeof(buffer));
                n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
                buffer[n] = '\0';
                printf("Received response: %s\n", buffer);
                if (strcmp(buffer, "ERROR: UNHANDLED REQUEST") == 0) {
                    printf("ERROR: wrong request sent from client\n");
                    break;
                }
                fprintf(fptr, "%s", buffer);
                word++;
            } while (strcmp(buffer, "FINISH") != 0 && strcmp(buffer, "FINISH\n") != 0);
            printf("Received file from server written to %s\n",newFile);
            free(request);
            fclose(fptr);
        }
        else{
            printf("HELLO MESSAGE NOT FOUND SERVER STOPPED RECIEVING \n");

        }
        
    }
    close(sockfd);
    return 0;
}