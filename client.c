#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include "buffer.h"

#define HOST "34.241.4.235"
#define PORT 8080

//cu aceasta functie colectez informatiile despre utilizator
char** get_user_info(char* username, char* password){
    printf("username=");
    scanf("%s", username);
    JSON_Value* value = json_value_init_object();
    JSON_Object* object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    printf("password=");
    scanf("%s", password);
    json_object_set_string(object, "password", password);
    char** info = malloc(1);
    char* serialized_string = json_serialize_to_string(value);
    info[0] = malloc(strlen(serialized_string));
    stpcpy(info[0], serialized_string);
    return info;
}
//inregistrez un client la server
void register_to_server(int sockfd, char* input, char* url, char* host, char* username, char* password){
    char** serialized_string = get_user_info(username, password);
    char* buffer = compute_post_request(host, url, "application/json", serialized_string, 1, NULL, 0, NULL);
    send_to_server(sockfd, buffer);
    char* receive_message = receive_from_server(sockfd);
    input = basic_extract_json_response(receive_message);
    if(input == NULL)
        printf("User registered!\n");
    else
        printf("\n%s\n", input);
    
}

//clientul se logheaza la server
char* login(int sockfd, char* input, char* url, char* host, char* username, char* password){
    char** serialized_string = get_user_info(username, password);
    char* buffer = compute_post_request(host, url, "application/json", serialized_string, 1, NULL, 0, NULL);
    send_to_server(sockfd, buffer);
    char* receive_message = receive_from_server(sockfd);
    //stochez cookies
    char* cookies = strstr(receive_message, "connect.sid");
    char* cookie = calloc(500, sizeof(char));
    //daca nu sunt cookies inseamna ca nu a reusit logarea
    if(cookies == NULL){
        printf("Error while login\n");
    }
    else{
        strtok(cookies, ";");
        strcpy(cookie, cookies);
        printf("Logged in\n");
    }
    //daca serverul intoarce un mesaj de eroare, returnez NULL, altfel cookies
    if(strstr(receive_message, "{") != NULL)
        return NULL;
    return cookie;
}
//cerere acces la biblioteca
char* library_access(int sockfd, char* host, char* url, char** cookies, int cookies_count){
    //daca nu am cookies inseamna ca nici nu sunt logat
    if(cookies == NULL)
        printf("Error! Please login before you try to get library access\n");
    char* message = compute_get_request(host, url, NULL, cookies, cookies_count, NULL);

    send_to_server(sockfd, message);
    char* receive_message = receive_from_server(sockfd);
    //stochez tokenul
    char* token = strstr(receive_message, "token");

    if(token != NULL){
        printf("You have access in library\n");
        token += 8;
        token[strlen(token) - 2] = '\0';
    }
    else
        printf("No access\n");
    return token;
}

void get_books(int sockfd, char* host, char* url, char** cookies, int cookies_count, char* token){
    char* message = compute_get_request(host, url, NULL, NULL, cookies_count, token);
    send_to_server(sockfd, message);
    char* receive_message = receive_from_server(sockfd);
    //daca serverul imi intoarce eroare, o afisez
    if(strstr(receive_message, "{\"error\":")){
        printf("%s\n", strstr(receive_message, "{\"error\":"));
        return;
    }
    //daca nu am eroare, imi intoarce lista si o afisez
    //gasind simbolul cu care se incepe lista "["
    printf("%s\n", strstr(receive_message, "["));
}

void get_book(int sockfd, char* host, char** cookies, int cookies_count, char* token){
    char* url = calloc(100, sizeof(char));
    int id;
    printf("id=");
    scanf("%d", &id);
    //formez ruta de acces
    sprintf(url, "/api/v1/tema/library/books/%d", id);
    char* message = compute_get_request(host, url, NULL, NULL, cookies_count, token);

    send_to_server(sockfd, message);
    char* receive_message = receive_from_server(sockfd);
    //afisez informatiile despre carte
    if(strstr(receive_message, "[") != NULL)
        printf("%s\n", strstr(receive_message, "["));
    if(strstr(receive_message, "{\"error\":") != NULL)
        printf("%s\n", strstr(receive_message, "{\"error\":"));
}

void add_book(int sockfd, char* input, char* url, char* host, char* authorization){
    char* title = calloc(100, sizeof(char));
    char* author = calloc(100, sizeof(char));
    char* genre = calloc(100, sizeof(char));
    char* publisher = calloc(100, sizeof(char));
    char* pages = calloc(10, sizeof(char));
    JSON_Value *value = json_value_init_object();
	JSON_Object *object = json_value_get_object(value);
    printf("title=");
    fgets(title, 50, stdin);
    if(title[0] == '\n'){
        printf("Wrong format\n");
        return;
    }
    //elimin \n de la sfarsit
    title[strlen(title) - 1] = '\0';
    //formez obiectul json
    json_object_set_string(object, "title", title);

    printf("author=");
    fgets(author, 50, stdin);
    if(author[0] == '\n'){
        printf("Wrong format\n");
        return;
    }
    author[strlen(author) - 1] = '\0';
    json_object_set_string(object, "author", author);

    printf("genre=");
    fgets(genre, 50, stdin);
    if(genre[0] == '\n'){
        printf("Wrong format\n");
        return;
    }
    genre[strlen(genre) - 1] = '\0';
    json_object_set_string(object, "genre", genre);

    printf("publisher=");
    fgets(publisher, 50, stdin);
    if(publisher[0] == '\n'){
        printf("Wrong format\n");
        return;
    }
    publisher[strlen(publisher) - 1] = '\0';
    json_object_set_string(object, "publisher", publisher);

    printf("pages=");
    fgets(pages, 50, stdin);
    //verific ca inputul pentru pagini sa fie cifre
    for(int i = 0; i < strlen(pages) - 1; i++){
        if(!isdigit(pages[i])){
            printf("Wrong format\n");
            return;
        }
    }
    int pages_ = atoi(pages);
    if(pages_ < 1){
        printf("Wrong format\n");
        return;
    }
    json_object_set_number(object, "page_count", pages_);
    char* serialized_string = json_serialize_to_string(value);
    char** books = malloc(1);
    books[0] = malloc(strlen(serialized_string));
    strcpy(books[0], serialized_string);
    //intr-un char** introduc cartea formata si o dau ca parametru lui post_request
    char* message = compute_post_request(host, url, "application/json", books, 1, NULL, 0, authorization);
    send_to_server(sockfd, message);
    char* receive_message = receive_from_server(sockfd);
    //afisez eroare daca exista
    if(strstr(receive_message, "{") == NULL)
        printf("The book is succesfully added\n");
    if(strstr(receive_message, "{\"error\":") != NULL)
        printf("%s\n", strstr(receive_message, "{\"error\":"));

}

void delete_book(int sockfd, char* host, char** cookies, int cookies_count, char* token){

    char* id = calloc(10, sizeof(char));
    printf("id=");
    fgets(id, 50, stdin);
    for(int i = 0; i < strlen(id) - 1; i++){
        if(!isdigit(id[i])){
            printf("Wrong format\n");
            return;
        }
    }
    int id_ = atoi(id);
    if(id_ < 0){
        printf("Wrong format\n");
        return;
    }

    char* url = calloc(100, sizeof(char));
    //formez ruta de acces
    sprintf(url, "/api/v1/tema/library/books/%d", id_);
    char* message = compute_delete_request(host, url, NULL, NULL, cookies_count, token);
    send_to_server(sockfd, message);
    char* receive_message = receive_from_server(sockfd);

    if(strstr(receive_message, "{") == NULL)
        printf("The book is succesfully deleted\n");
    else
        printf("%s\n", strstr(receive_message, "{\"error\":"));
}

void logout(int sockfd, char* host, char* url, char** cookies, int cookies_count, char* token){
    char* message = compute_get_request(host, url, NULL, cookies, cookies_count, token);
    send_to_server(sockfd, message);
    char* receive_message = receive_from_server(sockfd);
    if(strstr(receive_message, "{") == NULL)
        printf("You are logout\n");

}

int main(int argc, char *argv[]){
    int sockfd;
    char* input = calloc(100, sizeof(char));
    char host[16] = HOST;
    char* username = calloc(100, sizeof(char));
    char* password = calloc(100, sizeof(char));
    char* cookie = calloc(500, sizeof(char));
    char** cookies = NULL;
    
    char* token;
    int login_once = 0;

    while(1){
        //fac conexiune cu serverul pentru orice input
        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

        scanf("%s", input);

        if(strncmp(input, "register", 8) == 0){
            register_to_server(sockfd, input, "/api/v1/tema/auth/register", host, username, password);
        }
        else if(strncmp(input, "login", 5) == 0){
            //ma asigur ca numai o data ma pot loga
            if(login_once == 0){
                cookie = login(sockfd, input, "/api/v1/tema/auth/login", host, username, password);
                if(cookie != NULL){
                    cookies = malloc(1);
                    login_once = 1;
                    cookies[0] = malloc(strlen(cookie));
                    strcpy(cookies[0], cookie);
                }
            }
            else
                printf("You are logged in\n");
        }
        else if(strncmp(input, "enter_library", 13) == 0){
            token = library_access(sockfd, host, "/api/v1/tema/library/access", cookies, 1);
        }
        else if(strncmp(input, "get_books", 9) == 0){
            get_books(sockfd, host, "/api/v1/tema/library/books", NULL, 0, token);
        }
        else if(strncmp(input, "get_book", 8) == 0){
            get_book(sockfd, host, NULL, 0, token);
        }
        else if(strncmp(input, "add_book", 8) == 0){
            getchar();
            add_book(sockfd, input, "/api/v1/tema/library/books", host, token);
        }
        else if(strncmp(input, "delete_book", 11) == 0){
            getchar();
            delete_book(sockfd, host, NULL, 0, token);
        }
        else if(strncmp(input, "logout", 6) == 0){
            //ma asigur ca nu am erori in program daca dau de 2 ori logout
            if(login_once == 0){
                printf("You are already logout\n");
                continue;
            }
            logout(sockfd, host, " /api/v1/tema/auth/logout", cookies, 1, NULL);
            login_once = 0;
            if(cookie != NULL){
                free(cookie);
                cookie = NULL;
            }
            if(token != NULL)
                token = NULL;
            
        }
        else if(strncmp(input, "exit", 4) == 0)
            break;
    }
}