#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define OUTPUT_FILE_NAME "output.png"

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

int main(int argc, char* argv[]) {
    
    char* server_ip;
    int port;
    int sockfd;
    FILE *output_log, *output_file;
    struct sockaddr_in serv_addr, local_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    struct fake_tcp_header tcp_header1, tcp_header2, tcp_header3;
    int stop_running = 0;

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
        if (strcmp(argv[1], "localhost") == 0)
        {
            server_ip = "127.0.0.1";
        }
        else
        {
            server_ip = argv[1];
        }
        
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

    /* Create tcp_header1 */
    if (!stop_running)
    {
        addr_len = sizeof(local_addr);
        if (getsockname(sockfd, (struct sockaddr*)&local_addr, &addr_len) == 0) {
            
            tcp_header1.source_port = ntohs(local_addr.sin_port);
            tcp_header1.destination_port = htons(port);
            tcp_header1.seq_num = htonl(1); /* CHANGE_ME */
            tcp_header1.ack_num = htonl(0);
            tcp_header1.flags = htons(0x0002);  /* Syn only */
            tcp_header1.window_size = htons(17520);
            tcp_header1.checksum = htons(0xffff);
            tcp_header1.urgent_pointer = htons(0);
        }
        else {
            printf("getsockname failed\n");
            fprintf(output_log, "getsockname failed\n");
            stop_running = 1;
        }
    }

    /* Send tcp_header1 to the server */
    if (!stop_running) {
        if (send(sockfd, &tcp_header1, sizeof(tcp_header1), 0) < 0) {
            printf("Error sending tcp_header1\n");
            fprintf(output_log, "Error sending tcp_header1\n");
            stop_running = 1;
        }
        else {
            printf("tcp_header1 sent successfully.\n");
            fprintf(output_log, "tcp_header1 sent successfully.\n");
        }
    }

    /* Receive tcp_header2 from the server */
    if (!stop_running) {
        bytes_received = recv(sockfd, &tcp_header2, sizeof(tcp_header2), 0);
        if (bytes_received < 0) {
            printf("Error receiving tcp_header2\n");
            fprintf(output_log, "Error receiving tcp_header2\n");
            stop_running = 1;
        }
        else if (bytes_received == 0) {
            printf("Server closed connection.\n");
            fprintf(output_log, "Server closed connection.\n");
            stop_running = 1;
        }
        else {
            printf("tcp_header2 received successfully.\n");
            fprintf(output_log, "tcp_header2 received successfully.\n");
        }
    }

    if (!stop_running)
    {
        /* Create tcp_header3 */
        tcp_header3.source_port = tcp_header2.destination_port;
        tcp_header3.destination_port = tcp_header2.source_port;
        tcp_header3.seq_num = tcp_header2.ack_num;
        tcp_header3.ack_num = tcp_header2.seq_num + htons(1);
        tcp_header3.flags = htons(0x0010); /* Set the ACK flag */
        tcp_header3.window_size = htons(17520);
        tcp_header3.checksum = htons(0xffff);
        tcp_header3.urgent_pointer = htons(0);;

        /* Send tcp_header 3 */
        if (send(sockfd, &tcp_header3, sizeof(tcp_header3), 0) < 0) {
            printf("Failed to send tcp_header3\n");
            fprintf(output_log, "Failed to send tcp_header3\n");
            stop_running = 1;
        }
        else {
            printf("tcp_header3 sent successfully.\n");
            fprintf(output_log, "tcp_header3 sent successfully.\n");
        }
    }

    if (!stop_running)
    {
        /* Receive data */
        while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
            fwrite(buffer, 1, bytes_received, output_file);
        }

        printf("File received and saved to %s\n", OUTPUT_FILE_NAME);
        fprintf(output_log, "File received and saved to %s\n", OUTPUT_FILE_NAME);
    }
    
    if (output_log != NULL) { fclose(output_log); }
    if (output_file != NULL) { fclose(output_file); }

    if (sockfd >= 0)
    {
        close(sockfd);
    }

    return 0;
}