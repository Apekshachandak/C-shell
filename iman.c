#include "iman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_text_content(const char *html_content)
{
    const char *ptr = html_content;
    while (*ptr)
    {
        if (*ptr == '<')
        {
            while (*ptr && *ptr != '>') ptr++; // Skip HTML tags
            if (*ptr) ptr++; // Skip the closing '>'
        }
        else
        {
            putchar(*ptr); // Print text content
            ptr++;
        }
    }
}

void iMan(const char *input)
{
    const char *hostname = "man.he.net"; // hostname
    struct hostent *server = gethostbyname(hostname); 
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    // Extract the first argument (command_name) from the input
    char input_copy[256];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0';

    char *command_name = strtok(input_copy, " ");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    // Setting up the serve address structure
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); // HTTP port
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR connecting");
    }

    // Formulating the HTTP GET reuqest
    char request[1024];
    snprintf(request, sizeof(request), "GET /?topic=%s&section=all HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", command_name, hostname);

    // Sending the request
    int n = write(sockfd, request, strlen(request));
    if (n < 0)
    {
        error("ERROR writing to socket");
    }

    // Reading the responses
    char buffer[4096];
    char response[65536];
    memset(response, 0, sizeof(response));
    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[n] = '\0';
        strcat(response, buffer);
    }
    if (n < 0)
    {
        error("ERROR reading from socket");
    }

    close(sockfd); // Closingg the socket

    // Checking if the response we got indicates no matches found
    if (strstr(response, "No matches for") != NULL)
    {
        // Print out the entire response and removing the  HTML tags
        printf("ERROR: No matches for \"%s\" command\n", command_name);
        print_text_content(response);
         printf("ERROR: No matches for \"%s\" command\n", command_name);
        return;
    }

    // Finding the start of HTML content so that we can
    char *html_start = strstr(response, "<pre>");
    if (html_start == NULL)
    {
        html_start = strstr(response, "<PRE>");
        if (html_start == NULL)
        {
            printf("Error: Start of HTML content not found\n");
            return;
        }
    }

    // Find the end of HTML content
    char *html_end = strstr(html_start, "</pre>");
    if (html_end == NULL)
    {
        html_end = strstr(html_start, "</PRE>");
        if (html_end == NULL)
        {
            printf("Error: End of HTML content not found\n");
            return;
        }
    }

    printf("Man page for %s:\n", command_name);

    // Print content between <pre> tags
    html_start = strstr(html_start, ">");
    if (html_start == NULL)
    {
        printf("Error: Start of content inside <pre> not found\n");
        return;
    }
    html_start++;

    // Loop through the content between <pre> and </pre>
    while (html_start < html_end)
    {
        if (*html_start == '<')
        {
            while (*html_start != '>') html_start++;
            html_start++;
        }
        else
        {
            putchar(*html_start);
            html_start++;
        }
    }
    putchar('\n');
}
