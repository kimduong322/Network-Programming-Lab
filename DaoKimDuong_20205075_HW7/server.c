// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// #define MAX_users 5

// struct Client
// {
//     int socket;
//     char username[50];
//     char password[50];
//     int authenticated; 
//     int loginAtemps;
// };

// struct Client users[MAX_users];
// int num_users = 0;

// void doit(int client) {
//     // Listen from client
//     // if client choose Login
//     // recive username and password from client
//     // if (username's authenticated = 1) then sendto Client "user is logging in, can not login again"
//     // else if (username not in users[MAX_users]) then sento client "user not exist"
//     // else if (username in users[MAX_users] and isBlocked = 1) then sendto client "User not already"
//     // else if (username in users[MAX_users] and isBlocked = 0) and password is wrong and username's loginAtemps < 3) then (sendto client "Password incorrect" and loginAtemps++)
//     // else if username in users[MAX_users] and isBlocked = 0 and password is wrong and username's loginAtemps >= 3) then (sendto client "Account is blocked" and isBlocked = 1)
//     // else (sendto client "Login successful" and loginAttemps reset to 0)

//     // if client choose Logout
//     // recive username from client
//     // if username's authenticated = 1 then (sento Client "Bye username!" and reset username's authenticate = 0)
//     // else username's authenticated = 0 then sento Client "User name is not logging in! Cannot logout. Please login."
// }

// int main(int argc, char *argv[]) {
//     if (argc != 2)
//     {
//         fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }

//     int server_socket, client_socket;
//     struct sockaddr_in server_addr, client_addr;
//     socklen_t client_addr_len = sizeof(client_addr);

//     int port = atoi(argv[1]);

//     // Create socket
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket == -1)
//     {
//         perror("Error creating socket");
//         exit(EXIT_FAILURE);
//     }

//     // Set up server address structure
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(port);

//     // Bind socket to the address and port
//     if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
//     {
//         perror("Error binding socket");
//         exit(EXIT_FAILURE);
//     }
//     // Listen for incoming connections
//     if (listen(server_socket, MAX_users) == -1)
//     {
//         perror("Error listening for connections");
//         exit(EXIT_FAILURE);
//     }
//     printf("Server listening on port %d\n", port);

//     while (1)
//     {
//         // Accept
//         client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
//         if (client_socket == -1)
//         {
//             perror("Error accepting connection");
//             continue;
//         }

//         printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
//         // Fork a new process to handle the client
//         pid_t pid = fork();
//         if (pid == 0)
//         {
//             close(server_socket);
//             doit(client_socket);
//             close(client_socket);
//             exit(EXIT_SUCCESS);
//         }
        
//     }
//     // Close the server socket
//     close(server_socket);

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_USERS 100

struct User
{
    char username[50];
    char password[50];
    int authenticated;
    int loginAttempts;
    int isBlocked;
};

struct User users[MAX_USERS];
int num_users = 0;

void load_users_from_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[150];
    while (fgets(line, sizeof(line), file))
    {
        sscanf(line, "%s %s %d", users[num_users].username, users[num_users].password, &users[num_users].isBlocked);
        users[num_users].authenticated = 0;
        users[num_users].loginAttempts = 0;
        num_users++;
    }

    fclose(file);
}

void handle_client(int client_socket)
{
    // Receive menu choice, username, and password from the client
    char client_message[150];
    int byterecieved = recv(client_socket, client_message, sizeof(client_message), 0);

    if (byterecieved == -1)
    {
        perror("Error receiving data from client");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Ensure null-termination
    client_message[byterecieved] = '\0';

    char menu_choice[10];
    char client_username[50];
    char client_password[50];
    sscanf(client_message, "%s %s %s", menu_choice, client_username, client_password);

    if (strcmp(menu_choice, "login") == 0)
    {
        // Option: Login
        // Check if the username is already authenticated
        for (int i = 0; i < num_users; ++i)
        {
            if (strcmp(users[i].username, client_username) == 0 && users[i].authenticated)
            {
                printf("User %s is already logged in. Cannot log in again.\n", client_username);
                send(client_socket, "User is already logged in. Closing connection.\n", sizeof("User is already logged in. Closing connection.\n"), 0);
                return;
            }
        }

        // Check if the username exists in users[MAX_USERS]
        int user_exists = 0;
        for (int i = 0; i < num_users; ++i)
        {
            if (strcmp(users[i].username, client_username) == 0)
            {
                user_exists = 1;
                // Check if the user is already blocked
                if (users[i].isBlocked)
                {
                    printf("User %s is blocked. Cannot log in.\n", client_username);
                    send(client_socket, "User is blocked. Cannot log in.\n", sizeof("User is blocked. Cannot log in.\n"), 0);
                    return;
                }

                // Check login attempts and password
                if (users[i].loginAttempts >= 3)
                {
                    printf("User %s is blocked due to too many login attempts.\n", client_username);
                    send(client_socket, "Account is blocked. Too many login attempts.\n", sizeof("Account is blocked. Too many login attempts.\n"), 0);
                    users[i].isBlocked = 1;
                    return;
                }
                else
                {
                    if (strcmp(users[i].password, client_password) == 0)
                    {
                        users[i].authenticated = 1;
                        users[i].loginAttempts = 0;
                        printf("User %s logged in successfully.\n", client_username);
                        send(client_socket, "Login successful. You can now send messages.\n", sizeof("Login successful. You can now send messages.\n"), 0);
                        return;
                    }
                    else
                    {
                        users[i].loginAttempts++;
                        printf("Incorrect password for user %s. Login attempts: %d\n", client_username, users[i].loginAttempts);
                        send(client_socket, "Incorrect password. Please try again.\n", sizeof("Incorrect password. Please try again.\n"), 0);
                        return;
                    }
                }
            }
        }

        // If the username doesn't exist in users[MAX_USERS]
        if (!user_exists)
        {
            printf("User %s does not exist.\n", client_username);
            send(client_socket, "User does not exist.\n", sizeof("User does not exist.\n"), 0);
        }
    }
    else if (strcmp(menu_choice, "logout") == 0)
    {
        // Option: Logout
        // Check if the username is authenticated
        for (int i = 0; i < num_users; ++i)
        {
            if (strcmp(users[i].username, client_username) == 0 && users[i].authenticated)
            {
                printf("User %s logged out.\n", client_username);
                send(client_socket, "Logout successful. You can now log in again.\n", sizeof("Logout successful. You can now log in again.\n"), 0);
                users[i].authenticated = 0;
                close(client_socket);
                return;
            }
        }

        // If the username is not authenticated
        printf("User %s is not logged in. Cannot log out.\n", client_username);
        send(client_socket, "User is not logged in. Cannot log out. Please log in first.\n", sizeof("User is not logged in. Cannot log out. Please log in first.\n"), 0);
    }
    else
    {
        // Invalid menu choice
        printf("Invalid menu choice. Closing connection.\n");
        send(client_socket, "Invalid menu choice. Closing connection.\n", sizeof("Invalid menu choice. Closing connection.\n"), 0);
        close(client_socket);
    }
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("Loading user.......\n");
    load_users_from_file("account.txt");

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int port = atoi(argv[1]);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_USERS) == -1)
    {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1)
    {
        // Accept a connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1)
        {
            perror("Error accepting connection");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Fork a new process to handle the client
        pid_t pid = fork();
        if (pid == 0)
        {
            close(server_socket);
            handle_client(client_socket);
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0)
        {
            close(client_socket);
        }
        else
        {
            perror("Error forking process");
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
