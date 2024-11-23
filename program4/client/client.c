#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define OUTPUT_FILE_NAME "output.png"

int main(int argc, char* argv[]) {

    int stop_running = 0;
    char* server_ip;
    int port;
    int sockfd;
    FILE *output_log, *output_file;
    struct sockaddr_in serv_addr;

    /* Open output log file in append mode */
    output_log = fopen("output_client.txt", "a");

    if (output_log == NULL) {
        printf("Error opening output_client.txt\n");
        stop_running = 1;
    }
    
    if (argc < 3) 
    {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        fprintf(output_log, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        stop_running = 1;
    }
    else {
        /* Get server IP and port number */
        server_ip = argv[1];
        port = atoi(argv[2]);
        if (port <= 0)
        {
            printf("Invalid port number: %d\n", port);
            fprintf(output_log, "Invalid port number: %d\n", port);
            stop_running = 1;
        }
    }

    /* Open output file in binary write mode */
    if (!stop_running) 
    {
        output_file = fopen(OUTPUT_FILE_NAME, "wb");

        if (!output_file) {
            printf("Error opening %s\n", OUTPUT_FILE_NAME);
            fprintf(output_log, "Error opening %s\n", OUTPUT_FILE_NAME);
            stop_running = 1;
        }
    }

    /* Create socket (IPv4, TCP) */
    if (!stop_running)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Error creating socket\n");
            fprintf(output_log, "Error creating socket\n");
            stop_running = 1;
        }
    }

    if (!stop_running)
    {
        /* Zero out serv_addr struct and configure */
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET; /* IPv4 */
        serv_addr.sin_port = htons(port); /* Port */

        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            printf("Invalid address\n");
            fprintf(output_log, "Invalid address\n");
            close(sockfd);
            stop_running = 1;
        }
    }
    
    /* Connect to the server */ 
    if (!stop_running)
    {
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("Connection failed\n");
            fprintf(output_log, "Connection failed\n");
            close(sockfd);
            stop_running = 1;
        }
    }

    /* Receive data */
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, output_file);
    }

    fclose(output_file);
    
    if (sockfd >= 0)
    {
        close(sockfd);
    }

    printf("File received and saved to %s\n", OUTPUT_FILE_NAME);
    fprintf(output_log, "File received and saved to %s\n", OUTPUT_FILE_NAME);
    fclose(output_log);

    return 0;
}