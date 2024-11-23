#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BACKLOG 1
#define INPUT_FILE_NAME "input_img.png"
#define BUFFER_SIZE 1024

/**
* NAME : Michael Nakagawa
*
* HOMEWORK : 4
*
* CLASS : ICS 451
*
* INSTRUCTOR : Ravi Narayan
*
* DATE : October 13, 2024
*
* FILE : server.c
*
* DESCRIPTION : This file contains the server for hw4.
*               User is expected to provide port number as a command line argument.
*               Server will listen for connections, send a simple message to the client,
*               close the connection, and continue listening.
*               Server logs to output_server.txt.
*               Currently, the server will run infinitely.
**/

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    int stop_running;
    int port;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    FILE *output_file, *input_file;
    char buffer;

    stop_running = 0;

    /* Open output log file in append mode */
    output_file = fopen("output_server.txt", "a");

    if (output_file == NULL) {
        printf("Error opening output_server.txt\n");
        stop_running = 1;
    }

    /* Check if user provided port number */
    if (!stop_running && argc < 2)
    {
        printf("Usage: ./server <port>\n");
        fprintf(output_file, "Usage: ./server <port>\n");
        fclose(output_file);
        stop_running = 1;
    }
    /* Get port number */
    else
    {
        port = atoi(argv[1]);
        if (port <= 0)
        {
            printf("Invalid port number: %d\n", port);
            fprintf(output_file, "Invalid port number: %d\n", port);
            fclose(output_file);
            stop_running = 1;
        }
    }

    /* Create socket(IPv4, TCP) */ 
    if (!stop_running)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            printf("Error creating socket\n");
            fprintf(output_file, "Error creating socket\n");
            fclose(output_file);
            stop_running = 1;
        }
    }
    
    
    if (!stop_running)
    {
        /* Zero out serv_addr struct and configure */
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET; /* IPv4 */ 
        serv_addr.sin_addr.s_addr = INADDR_ANY; /* Bind to all interfaces */
        serv_addr.sin_port = htons(port); /* Port */ 

        /* Bind socket to the address and port */
        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("Error binding socket\n");
            fprintf(output_file, "Error binding socket\n");
            fclose(output_file);
            stop_running = 1;
        }
    }

    /* Listen for connections */ 
    if (!stop_running)
    {
        if (listen(sockfd, BACKLOG) < 0)
        {
            printf("Error listening on socket\n");
            fprintf(output_file, "Error listening on socket\n");
            stop_running = 1;
        }
        else
        {
            printf("Server is listening on port %d\n", port);
            fprintf(output_file, "Server is listening on port %d\n", port);
        }
    }

    fclose(output_file);

    /* Loop to accept and handle clients */
    while (!stop_running) 
    {
        output_file = fopen("output_server.txt", "a");

        if (output_file == NULL) {
            printf("Error opening output_server.txt\n");
            stop_running = 1;
        }

        memset(&cli_addr, 0, sizeof(cli_addr));
        clilen = sizeof(cli_addr);

        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            printf("Error accepting connection\n");
            fprintf(output_file, "Error accepting connection\n");
            fclose(output_file);
            stop_running = 1;
        }

        if (!stop_running)
        {
            printf("Accepted connection from client: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            fprintf(output_file, "Accepted connection from client: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

            printf("Sending message to client\n");
            fprintf(output_file, "Sending message to client\n");
            write(newsockfd, MESSAGE, strlen(MESSAGE));

            /* Open image file */
            input_file = fopen(INPUT_FILE_NAME, "rb");
            if (input_file == NULL) {
                printf("Error opening image file: %s\n", INPUT_FILE_NAME);
                fprintf(output_file, "Error opening image file: %s\n", INPUT_FILE_NAME);
                close(newsockfd);
                fclose(output_file);
                stop_running = 1;
            }

            if (!stop_running)
            {
                /* Read file and send to client */
                buffer[BUFFER_SIZE];
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, input_file)) > 0) {
                    send(newsockfd, buffer, bytes_read, 0);
                }
            }
            
            close(newsockfd);
            printf("Closed connection to client\n");
            fprintf(output_file, "Closed connection to client\n");
        }

        fclose(output_file);
    }
    
    if (sockfd >= 0)
    {
        close(sockfd);
    }

    return 0;
}