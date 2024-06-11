#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "parson.h"
#include "requests.h"
#include <ctype.h>

#define SIZE 90
// Server address
#define ADDR "34.246.184.49"
// Server port
#define PORT 8080

// defines for paths
#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define LOGOUT "/api/v1/tema/auth/logout"
#define ACCESS_LIBRARY "/api/v1/tema/library/access"
#define LIBRARY_BOOKS "/api/v1/tema/library/books"
#define LIBRARY_BOOK "/api/v1/tema/library/books/"
#define TYPE "application/json"

// Function to prompt for a command input
void prompt_for_command(char *command) {
    fgets(command, 150, stdin);
    command[strcspn(command, "\n")] = 0;
}

// Function to prompt for user credentials
void credentials_prompt(char *username, char *password) {
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);
}

// Function to free allocated memory for cookies
void free_cookies(char **cookies, int cookiesSize) {
    for (int i = 0; i < cookiesSize; i++) {
        free(cookies[i]);
    }
}

int main() {
    int sockfd;                          // Socket file descriptor
    char command[30];                    // Command input
    char username[300];                  // Username input
    char password[300];                  // Password input
    char id[30];                         // Book ID input
    char *message;                       // Message to be sent to server
    char *response;                      // Response received from server

    char **cookies = malloc(100 * sizeof(char *)); // Array of cookies
    for (int i = 0; i < 100; i++) {
        cookies[i] = malloc(1000);
    }

    // Number of stored cookies
    int cookiesSize = 0;

    JSON_Value *jsonVal = json_value_init_object();     // JSON value object
    JSON_Object *jsonObj = json_value_get_object(jsonVal);    // JSON object
    char *token = NULL;                                    // JSON Web Token

    // Prompt user for initial command
    prompt_for_command(command);

    // Main loop to handle commands until getting "exit"
    while (strcmp(command, "exit") != 0) {
        // REGISTER COMMAND
        if (strcmp(command, "register") == 0) {
            if (cookiesSize > 0) {
                printf("ERROR : You are already logged in. Log out first!\n");
                prompt_for_command(command);
                continue;
            }

            // Open connection to server
            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);
            // Prompt for credentials
            credentials_prompt(username, password);
            // Set username in JSON          
            json_object_set_string(jsonObj, "username", username);
            // Set password in JSON
            json_object_set_string(jsonObj, "password", password);

            // Serialize JSON object to string
            char *jsonString = json_serialize_to_string(jsonVal);
            // Create and send POST request for registration
            message = compute_post_request(ADDR, REGISTER, TYPE,
                            &jsonString, strlen(jsonString), NULL, 0, token);

            send_to_server(sockfd, message);

            response = receive_from_server(sockfd); // Receive server response
            close_connection(sockfd);               // Close connection
            json_free_serialized_string(jsonString); // Free JSON string

            // Check for errors in response
            if (strstr(response, "error") != NULL) {
                printf("ERROR : Username %s already exists!\n", username);
            } else {
                printf("SUCCESS : New user is registered!\n");
            }
            free(response); // Free response buffer
            prompt_for_command(command); // Prompt for next command
            continue;
        }

        // LOGIN COMMAND
        if (strcmp(command, "login") == 0) {
            if (cookiesSize > 0) {
                printf("ERROR : You are already logged in. Log out first!\n");
                prompt_for_command(command);
                continue;
            }

            // Open connection to server
            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);
            credentials_prompt(username, password);
            json_object_set_string(jsonObj, "username", username);
            json_object_set_string(jsonObj, "password", password);

            char *jsonString = json_serialize_to_string(jsonVal);
            // Create and send POST request for login
            message = compute_post_request(ADDR, LOGIN, TYPE,
                            &jsonString, strlen(jsonString), NULL, 0, token);
            // send message to server
            send_to_server(sockfd, message);

            response = receive_from_server(sockfd); // Receive server response
            close_connection(sockfd);                      // Close connection
            json_free_serialized_string(jsonString);       // Free JSON string

            // Check for errors in response
            if (strstr(response, "error") != NULL) {
                printf("ERROR : Wrong username or password!\n");
            } else {
                printf("SUCCESS : You are logged in now!\n");
                char *cookies_line = strstr(response, "Set-Cookie");
                if (cookies_line) {
                    cookies_line += 12;              // Skip "Set-Cookie: "
                    char *cookie = strtok(cookies_line, ";");
                    strcpy(cookies[cookiesSize++], cookie); // Store cookie
                }
            }

            free(response);              // Free response buff
            prompt_for_command(command); // Prompt for next command
            continue;
        }

        // ENTER_LIBRARY COMMAND
        if (strcmp(command, "enter_library") == 0) {
            if (cookiesSize == 0) {
                printf("ERROR : You are not logged in!\n");
                prompt_for_command(command);
                continue;
            }

            // Open connection to server
            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);
            // Create and send GET request for library access
            message = compute_get_request(ADDR, ACCESS_LIBRARY, NULL,
                                                 cookies, cookiesSize, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd); // Receive server response
            close_connection(sockfd);               // Close connection

            char *token_line = strstr(response, "{\"token\":\"");

            if (token_line) {
                token = malloc(1000); // Allocate memory for token
                strncpy(token, token_line + 10, strlen(token_line) - 12);
                printf("SUCCESS : You have access to the library!\n");
            }

            free(response);              // Free response buffer
            prompt_for_command(command); // Prompt for next command
            continue;
        }

        // GET_BOOKS COMMAND
        if (strcmp(command, "get_books") == 0) {
            if (token == NULL) {
                printf("ERROR : You don't have access to the library!\n");
                prompt_for_command(command);
                continue;
            }

            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);

            // Create and send GET request for list of books
            message = compute_get_request(ADDR, LIBRARY_BOOKS, NULL,
                                                cookies, cookiesSize, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd); // Receive server response

            // Close connection
            close_connection(sockfd);

            // Find the start of the data in the response
            char *data = strstr(response, "\r\n\r\n");
            if (data != NULL) {
                data += 4; // Skip the "\r\n\r\n"

                // Check for error messages in the response data
                if (strstr(data, "error") != NULL) {
                    printf("ERROR : %s\n", data);
                } else {
                    // Print the JSON response data
                    printf("%s\n", data);
                }
            } else {
                printf("ERROR: Could not parse the response.\n");
            }

            free(response);
            prompt_for_command(command);
            continue;
        }

        // GET_BOOK COMMAND
        if (strcmp(command, "get_book") == 0) {
            if (token == NULL) {
                printf("ERROR : You don't have access to library!\n");
                prompt_for_command(command);
                continue;
            }

            printf("id=");
            scanf("%s", id); // Read the ID
            char bookAddr[100] = LIBRARY_BOOK; // Create book address path
            strcat(bookAddr, id);

            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);
            // Create and send GET request for book details
            message = compute_get_request(ADDR, bookAddr, NULL, cookies,
                                                        cookiesSize, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close_connection(sockfd);

            printf("%s\n", response);    // Print book details
            free(response);              // Free response buffer
            prompt_for_command(command); // Prompt for next command
            continue;
        }

        // ADD_BOOK COMMAND
        if (strcmp(command, "add_book") == 0) {
            if (token == NULL) {
                printf("ERROR : You don't have access to library!\n");
                prompt_for_command(command);
                continue;
            }

            // Book details
            char title[90], author[90], genre[90], publisher[90], page_count[90];

            printf("title=");
            // read the entire line
            fgets(title, SIZE, stdin);
            // get rid of '\n'
            title[strcspn(title, "\n")] = 0;

            printf("author=");
            fgets(author, SIZE, stdin);
            author[strcspn(author, "\n")] = 0;

            printf("genre=");
            fgets(genre, SIZE, stdin);
            genre[strcspn(genre, "\n")] = 0;

            printf("publisher=");
            fgets(publisher, SIZE, stdin);
            publisher[strcspn(publisher, "\n")] = 0;

            printf("page_count=");
            fgets(page_count, SIZE, stdin);
            page_count[strcspn(page_count, "\n")] = 0;

            // if any of them are empty -> error
            if (strcmp(title, "") == 0 || strcmp(author, "") == 0 ||
                strcmp(genre, "") == 0 || strcmp(publisher, "") == 0) {
                printf("ERROR : Your data is empty!\n");
                prompt_for_command(command);
                continue;
            }

            // Validate page count
            int int_page_count = atoi(page_count);
            if (int_page_count == 0 && strcmp(page_count, "0") != 0) {
                printf("ERROR : Invalid type for page count!\n");
                prompt_for_command(command);
                continue;
            }

            // Set book details in JSON object
            json_object_set_string(jsonObj, "title", title);
            json_object_set_string(jsonObj, "author", author);
            json_object_set_string(jsonObj, "genre", genre);
            json_object_set_number(jsonObj, "page_count", int_page_count);
            json_object_set_string(jsonObj, "publisher", publisher);

            // Serialize JSON object to string
            char *jsonString = json_serialize_to_string(jsonVal);
            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);

            // Create and send POST request to add book
            message = compute_post_request(ADDR, LIBRARY_BOOKS,
            TYPE, &jsonString, strlen(jsonString), NULL, 0, token);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd); // Receive server response
            close_connection(sockfd);               // Close connection
            json_free_serialized_string(jsonString); // Free JSON string

            printf("SUCCESS : A new book was added successfully!\n");
            free(response);              // Free response buff
            prompt_for_command(command); // Prompt for next command
            continue;
        }

        // DELETE_BOOK COMMAND
        if (strcmp(command, "delete_book") == 0) {
            if (token == NULL) {
                printf("ERROR : You don't have access to library!\n");
                prompt_for_command(command);
                continue;
            }

            printf("id=");
            scanf("%s", id);                        // Read the book id
            char bookAddr[100] = LIBRARY_BOOK; // Create book addr path
            strcat(bookAddr, id);

            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);

            // Create and send DELETE request for book
            message = compute_delete_request(ADDR, bookAddr, NULL, cookies, cookiesSize, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close_connection(sockfd);

            // Check for errors in response
            if (strstr(response, "error") != NULL) {
                printf("ERROR : Book with id %s doesn't exist!\n", id);
            } else {
                printf("SUCCESS : Book with id %s was deleted!\n", id);
            }
            free(response);
            prompt_for_command(command); // next command
            continue;
        }

        // LOGOUT COMMAND
        if (strcmp(command, "logout") == 0) {
            if (cookiesSize == 0) {
                printf("ERROR : You are not logged in!\n");
                prompt_for_command(command);
                continue;
            }

            sockfd = open_connection(ADDR, PORT, AF_INET, SOCK_STREAM, 0);
            // Create and send GET request for logout
            message = compute_get_request(ADDR, LOGOUT, NULL, cookies,
                                                         cookiesSize, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close_connection(sockfd);

            // Reset cookies and token
            cookiesSize = 0;
            free(token);
            token = NULL;
            printf("SUCCESS : You logged out successfully!\n");
            free(response);
            prompt_for_command(command);
            continue;
        }

        prompt_for_command(command); // Prompt for next command
    }

    // free alocated memory
    free_cookies(cookies, cookiesSize); // Free cookies
    free(cookies);                      // Free cookies array
    json_value_free(jsonVal);           // Free JSON value

    return 0;
}
