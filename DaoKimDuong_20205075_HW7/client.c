// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// char loggingUser[50];

// int main(int argc, char *argv[]) {
//     if (argc != 3)
//     {
//         fprintf(stderr, "Usage: %s <ip_address> <port>\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }
//     int client_socket;
//     struct sockaddr_in server_addr;
//     // Create socket
//     client_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (client_socket == -1)
//     {
//         perror("Error creating socket");
//         exit(EXIT_FAILURE);
//     } 
//     // Set up server address structure
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = inet_addr(argv[1]);
//     server_addr.sin_port = htons(atoi(argv[2]));
//     // Connect to server
//     if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
//     {
//         perror("Error connecting to server");
//         exit(EXIT_FAILURE);
//     }
//     printf("Connected to server at %s:%s\n", argv[1], argv[2]);

//     char menu_choice[10];
//     char username[50];
//     char password[50];

//     while (1) {
//         // Display menu
//         printf("Menu:\n");
//         printf("1. Login\n");
//         printf("2. Logout\n");
//         printf("Choose an option (1 or 2): ");
//         fgets(menu_choice, sizeof(menu_choice), stdin);
//         if (strcmp(menu_choice, "1\n") == 0) {
//             printf("Enter username: ");
//             fgets(username, sizeof(username), stdin);
//             printf("Enter password: ");
//             fgets(password, sizeof(password), stdin);
//             // Send menu choice to the server
//             send(client_socket, "login\n", sizeof("login\n"), 0);
//             // Send username and password to the server
//             send(client_socket, username, strlen(username), 0);
//             send(client_socket, password, strlen(password), 0);
//             // Receive authentication result
//             // print result
//             // if result == "Login successful" then loggingUser = username
//         }
//         else if (strcmp(menu_choice, "2\n") == 0)
//         {
//             // if loggingUser is empty then printf("Not login. Please login first!")
//             // else sendto server "Logout loggingUser"
//         } 
//         else {
//             printf("Invalid menu choice. Try again.\n");
//         }
        
//     }
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char loggingUser[50];

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <ip_address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%s\n", argv[1], argv[2]);

    char menu_choice[10];
    char username[50];
    char password[50];

    while (1)
    {
        // Display menu
        printf("Menu:\n");
        printf("1. Login\n");
        printf("2. Logout\n");
        printf("Choose an option (1 or 2): ");
        fgets(menu_choice, sizeof(menu_choice), stdin);

        if (strcmp(menu_choice, "1\n") == 0)
        {
            printf("Enter username: ");
            fgets(username, sizeof(username), stdin);
            printf("Enter password: ");
            fgets(password, sizeof(password), stdin);

            if (strcmp(loggingUser, username) == 0) {
                printf("Only one account login permission failure!\n");
                continue;
            }
            // Send menu choice, username, and password to the server
            char message[150];
            sprintf(message, "login %s %s\n", username, password);
            send(client_socket, message, sizeof(message), 0);

            // Receive authentication result
            char result[100];
            recv(client_socket, result, sizeof(result), 0);
            printf("%s", result);

            // Update loggingUser if login successful
            if (strstr(result, "Login successful") != NULL)
            {
                strncpy(loggingUser, username, sizeof(loggingUser));
            }
        }
        else if (strcmp(menu_choice, "2\n") == 0)
        {
            // If loggingUser is empty then printf("Not logged in. Please log in first!")
            // Else send to server "logout loggingUser"
            if (strlen(loggingUser) == 0)
            {
                printf("Not logged in. Please log in first!\n");
            }
            else
            {
                // Send the logout message
                char logout_message[100];
                snprintf(logout_message, sizeof(logout_message), "logout %s abc\n", loggingUser);
                send(client_socket, logout_message, sizeof(logout_message), 0);

                // Receive logout result
                char result[100];
                recv(client_socket, result, sizeof(result), 0);
                printf("%s", result);

                // Reset loggingUser if logout successful
                if (strstr(result, "Logout successful") != NULL)
                {
                    memset(loggingUser, 0, sizeof(loggingUser));
                    close(client_socket);
                }
            }
        }
        else
        {
            printf("Invalid menu choice. Try again.\n");
        }
    }

    // Close the client socket
    close(client_socket);

    return 0;
}
