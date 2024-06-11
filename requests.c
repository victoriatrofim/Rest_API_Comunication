#include <stdlib.h>  
#include <stdio.h>
#include <unistd.h>  
#include <string.h>    
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>  
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);
    // add the host
    if (query_params != NULL) {
        sprintf(line, "Host: %s?%s", host, query_params);
    } else {
        sprintf(line, "Host: %s", host);
    }
    compute_message(message, line);

    // add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
       sprintf(line, "Cookie: %s", cookies[0]);
       compute_message(message, line);
    }

    if (JWT != NULL) {
        sprintf(line, "Authorization: Bearer %s", JWT);
       compute_message(message, line);
    }
    // add final new line
    compute_message(message, "");
    return message;
}


char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);
    if (query_params != NULL) {
        sprintf(line, "Host: %s?%s", host, query_params);
    } else {
        sprintf(line, "Host: %s", host);
    }
    compute_message(message, line);

    // add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
       sprintf(line, "Cookie: %s", cookies[0]);
       compute_message(message, line);
    }

    if (JWT != NULL) {
        sprintf(line, "Authorization: Bearer %s", JWT);
       compute_message(message, line);
    }
    // add final new line
    compute_message(message, "");
    return message;
}



char *compute_post_request(char *host, char *url, char* content_type, 
            char **body_data, int body_data_fields_count, char **cookies,
                                             int cookies_count, char *JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (JWT != NULL) {
        sprintf(line, "Authorization: Bearer %s", JWT);
       compute_message(message, line);
    }

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %d", body_data_fields_count);
    compute_message(message, line);
    compute_message(message, "");
    sprintf(line, "%s", body_data[0]);
    compute_message(message, line);
    /* add necessary headers (Content-Type and Content-Length are mandatory)
    in order to write Content-Length you must first compute the message size
    */
    // add cookies
    if (cookies != NULL) {
       
    }
    // add the actual payload data
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}
