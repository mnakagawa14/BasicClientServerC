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
* HOMEWORK : 8
*
* CLASS : ICS 451
*
* INSTRUCTOR : Ravi Narayan
*
* DATE : November 24, 2024
*
* FILE : server.c
*
* DESCRIPTION : This file contains the server for hw8.
*               User is expected to provide port number as a command line argument.
*               Server will listen for connections, simulates a three-way TCP handshake,
*               and sends a png.
*               Server logs to output_server.txt.
**/

/* struct to represent TCP header */
struct fake_tcp_header
{
    unsigned short source_port;
    unsigned short destination_port;
    unsigned long seq_num;
    unsigned long ack_num;
    unsigned short flags; /* Includes data offset and reserved bits */
    unsigned short window_size;
    unsigned short checksum;
    unsigned short urgent_pointer;
};

void print_header_hex(struct fake_tcp_header* header, FILE* output_file);
void print_tcp_header_details(struct fake_tcp_header* header, FILE* output_file);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    int stop_running;
    int port;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    FILE *output_file, *input_file;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_received;
    struct fake_tcp_header tcp_header1, tcp_header2, tcp_header3;
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

    /* Loop to accept and handle clients */
    while (!stop_running) 
    {
        /* Accept connection from client */
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
        }

        /* Receive tcp_header1 from client */
        if (!stop_running)
        {
            bytes_received = recv(newsockfd, &tcp_header1, sizeof(tcp_header1), 0);
            if (bytes_received < 0) {
                printf("Error receiving tcp_header1\n");
                fprintf(output_file, "Error receiving tcp_header1\n");
                stop_running = 1;
            }
            else if (bytes_received == 0) {
                printf("Client closed connection.\n");
                fprintf(output_file, "Client closed connection.\n");
                stop_running = 1;
            }
            else {
                printf("tcp_header1 received successfully.\n");
                fprintf(output_file, "tcp_header1 received successfully.\n");
                print_header_hex(&tcp_header1, output_file);
                print_tcp_header_details(&tcp_header1, output_file);
            }
        }

        if (!stop_running)
        {
            /* Create tcp_header2 */
            tcp_header2.source_port = tcp_header1.destination_port;
            tcp_header2.destination_port = tcp_header1.source_port;
            tcp_header2.seq_num = htonl((unsigned long) rand());
            tcp_header2.ack_num = htonl(ntohl(tcp_header1.seq_num) + 1);
            tcp_header2.flags = htons(0x0012); /* Set the SYN and ACK flag */
            tcp_header2.window_size = htons(17520);
            tcp_header2.checksum = htons(0xffff);
            tcp_header2.urgent_pointer = htons(0);

            /* Send tcp_header2*/
            if (send(newsockfd, &tcp_header2, sizeof(tcp_header2), 0) < 0) {
                printf("Failed to send tcp_header2\n");
                fprintf(output_file, "Failed to send tcp_header2\n");
                stop_running = 1;
            }
            else {
                printf("tcp_header2 sent successfully.\n\n");
                fprintf(output_file, "tcp_header2 sent successfully.\n\n");
            }
        }

        /* Receive tcp_header3 from the client */
        if (!stop_running) {
            bytes_received = recv(newsockfd, &tcp_header3, sizeof(tcp_header3), 0);
            if (bytes_received < 0) {
                printf("Error receiving tcp_header3\n");
                fprintf(output_file, "Error receiving tcp_header3\n");
                stop_running = 1;
            }
            else if (bytes_received == 0) {
                printf("Client closed connection.\n");
                fprintf(output_file, "Client closed connection.\n");
                stop_running = 1;
            }
            else {
                printf("tcp_header3 received successfully.\n");
                fprintf(output_file, "tcp_header3 received successfully.\n");
                print_header_hex(&tcp_header3, output_file);
                print_tcp_header_details(&tcp_header3, output_file);
            }
        }

        if (!stop_running)
        {
            
            printf("Sending file to client\n");
            fprintf(output_file, "Sending file to client\n");

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
                while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, input_file)) > 0) {
                    send(newsockfd, buffer, bytes_read, 0);
                }
            }

            if (sockfd >= 0)
            {
                close(sockfd);
            }

            printf("Closed connection to client\n");
            fprintf(output_file, "Closed connection to client\n");
            stop_running = 1;
        }

        fclose(output_file);
    }

    return 0;
}

/*
 * Helper function to print fake_tcp_header as hexadecimal
 * Prints to both console and output file.
 */
void print_header_hex(struct fake_tcp_header* header, FILE* output_file) {
    unsigned char* bytes = (unsigned char*)header;
    size_t header_size = sizeof(struct fake_tcp_header);
    size_t i;

    printf("Raw header:\n");
    fprintf(output_file, "Raw header:\n");

    for (i = 0; i < header_size; ++i) {
        printf("%02X ", bytes[i]);
        fprintf(output_file, "%02X ", bytes[i]);

        if ((i + 1) % 8 == 0) {
            printf("\n");
            fprintf(output_file, "\n");
        }
    }
    printf("\n");
    fprintf(output_file, "\n");
}

/*
 * Helper function to print TCP header details in decimal and binary
 * Prints to both the console and the output file.
 */
void print_tcp_header_details(struct fake_tcp_header* header, FILE* output_file) {
    unsigned short flags;
    
    /* Print to console */ 
    printf("TCP Header Details:\n");
    printf("Source Port: %u\n", ntohs(header->source_port));
    printf("Destination Port: %u\n", ntohs(header->destination_port));
    printf("Sequence Number: %u\n", ntohl(header->seq_num));
    printf("Acknowledgment Number: %u\n", ntohl(header->ack_num));
    printf("Control Bits: ");

    /* Print to file */
    fprintf(output_file, "TCP Header Details:\n");
    fprintf(output_file, "Source Port: %u\n", ntohs(header->source_port));
    fprintf(output_file, "Destination Port: %u\n", ntohs(header->destination_port));
    fprintf(output_file, "Sequence Number: %u\n", ntohl(header->seq_num));
    fprintf(output_file, "Acknowledgment Number: %u\n", ntohl(header->ack_num));
    fprintf(output_file, "Control Bits: ");

    /* Extract control bits */ 
    flags = ntohs(header->flags);
    if (flags & 0x0001) { printf("FIN "); fprintf(output_file, "FIN "); }
    if (flags & 0x0002) { printf("SYN "); fprintf(output_file, "SYN "); }
    if (flags & 0x0004) { printf("RST "); fprintf(output_file, "RST "); }
    if (flags & 0x0008) { printf("PSH "); fprintf(output_file, "PSH "); }
    if (flags & 0x0010) { printf("ACK "); fprintf(output_file, "ACK "); }
    if (flags & 0x0020) { printf("URG "); fprintf(output_file, "URG "); }
    if (flags & 0x0040) { printf("ECE "); fprintf(output_file, "ECE "); }
    if (flags & 0x0080) { printf("CWR "); fprintf(output_file, "CWR "); }

    printf("\n\n");
    fprintf(output_file, "\n\n");
}
