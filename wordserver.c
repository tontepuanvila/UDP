// ||Link of the pcap file: https://drive.google.com/drive/folders/1BT9e0kvHX6i-VzwbnAu_gIltqW5uKpJR?usp=drive_link            ||
// ++==========================================================================================================================++
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
    struct sockaddr_in servaddr, cliaddr;
    char *line = NULL;
    size_t llen = 0;
    ssize_t read;

    // Create socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.127.127.127");
    servaddr.sin_port = htons(5000);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server running...\n");

    int n;
    socklen_t len;
    char buffer[MAXLINE];

    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
    buffer[n] = '\0';
    printf("Requested file is %s\n", buffer);

    if (access(buffer, R_OK) != -1) {
        FILE *fptr = fopen(buffer, "r");
        if (fptr == NULL) {
            printf("Error opening file!\n");
            close(sockfd);
            exit(1);
        }

        char *req_begin = malloc(MAXLINE + 1);
        int word = 1;
        while ((read = getline(&line, &llen, fptr)) != -1) {
            printf("Sending line (length %zu): %s\n", read, line);
            sendto(sockfd, line, strlen(line), 0, (const struct sockaddr *) &cliaddr, len);
            if (strcmp(line, "FINISH") == 0 || strcmp(line, "FINISH\n") == 0) {
                printf("SUCCESS: all lines sent!\n");
                break;
            }
            memset(buffer, 0, sizeof(buffer));
            n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
            sprintf(req_begin, "WORD%d", word);
            if (strcmp(buffer, req_begin) != 0) {
                printf("ERROR: wrong request from client\nreceived: %s | expected: %s\n", buffer, req_begin);
                sendto(sockfd, "ERROR: UNHANDLED REQUEST", strlen("ERROR: UNHANDLED REQUEST"), 0, (const struct sockaddr *) &cliaddr, len);
                break;
            }
            word++;
        }
        
        free(req_begin);
        fclose(fptr);
        if (line) {
            free(line);
        }
    } else {
        char *begin = "NOTFOUND ";
        char *to_buffer = malloc(strlen(begin) + strlen(buffer) + 1);
        if (to_buffer == NULL) {
            printf("ERROR: OUT OF MEMORY!!!\n");
        } else {
            printf("ERROR: Requested file not found!\n");
            strcpy(to_buffer, begin);
            strcat(to_buffer, buffer);
            sendto(sockfd, (const char *)to_buffer, strlen(to_buffer), 0, (struct sockaddr *) &cliaddr, len);
            free(to_buffer);
        }
    }
    close(sockfd);
    return 0;
}