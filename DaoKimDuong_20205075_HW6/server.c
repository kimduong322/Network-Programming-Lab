#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MESSAGE_SIZE 1024

void md5Encode(const char *input, char *output)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, input, strlen(input));
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &ctx);

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(&output[i * 2], "%02x", (unsigned int)digest[i]);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <PortNumber>\n", argv[0]);
        exit(1);
    }
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("socket");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_socket);
        exit(1);
    }
    // Listen for incoming connections
    if (listen(server_socket, 2) == -1)
    {
        perror("listen");
        close(server_socket);
        exit(1);
    }

    printf("Server is listening for incoming connections...\n");
    while (1)
    {
        // Accept incoming connection
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1)
        {
            perror("accept");
            continue;
        }
        printf("You got a connection from %s in port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (1)
        {
            char request[MAX_MESSAGE_SIZE];
            ssize_t bytes_received = recv(client_socket, request, sizeof(request), 0);

            if (bytes_received <= 0)
            {
                // Client closed the connection or an error occurred
                close(client_socket);
                break;
            }

            // Process the client request
            request[bytes_received] = '\0';

            if (strstr(request, "STRING") != NULL)
            {
                // Process the string input as before
                char input[MAX_MESSAGE_SIZE];
                char alpha[MAX_MESSAGE_SIZE];
                char numeric[MAX_MESSAGE_SIZE];
                sscanf(request, "STRING %s", input);

                // Check if input contains only alphanumeric characters
                int isInvalidInput = 0;
                for (size_t i = 0; i < strlen(input); i++)
                {
                    if (!isalnum(input[i]))
                    {
                        isInvalidInput = 1;
                        break;
                    }
                }

                if (isInvalidInput)
                {
                    printf("Received and processed string: \n");
                    printf("Alphabetic: \n");
                    printf("Numeric: \n");
                    char errorMessage[MAX_MESSAGE_SIZE];
                    sprintf(errorMessage, "ERROR: Invalid request format");
                    int errorSent = send(client_socket, errorMessage, strlen(errorMessage), 0);
                    if (errorSent == -1)
                    {
                        perror("send");
                    }
                }
                else
                {
                    md5Encode(input, alpha);
                    int alphaIndex = 0, numericIndex = 0;
                    for (size_t i = 0; i < strlen(alpha); i++)
                    {
                        if (isalpha(alpha[i]))
                        {
                            alpha[alphaIndex++] = alpha[i];
                        }
                        else if (isdigit(alpha[i]))
                        {
                            numeric[numericIndex++] = alpha[i];
                        }
                    }
                    alpha[alphaIndex] = '\0';
                    numeric[numericIndex] = '\0';
                    printf("Received and processed string: %s\n", input);
                    printf("Alphabetic: %s\n", alpha);
                    printf("Numeric: %s\n", numeric);

                    char okMessage[] = "OK";
                    int okSent = send(client_socket, okMessage, strlen(okMessage), 0);
                    if (okSent == -1)
                    {
                        perror("send");
                    }
                }
            }
            else if (strstr(request, "IMAGE_NAME_AND_TYPE") != NULL)
            {
                printf("Processing Image\n");
                // Xử lý tên và kiểu file
                char filename[MAX_MESSAGE_SIZE];
                char file_type[MAX_MESSAGE_SIZE];
                sscanf(request, "IMAGE_NAME_AND_TYPE %s %s", filename, file_type);
                // Mở file để ghi nội dung
                // Tên thư mục chứa tệp
                const char *directory = "ServerImgSrc";
                char full_path[MAX_MESSAGE_SIZE * 2 + 1];
                snprintf(full_path, sizeof(full_path), "%s/%s.%s", directory, filename, file_type);
                FILE *file = fopen(full_path, "wb");

                if (file == NULL)
                {
                    printf("Không thể mở tệp %s để ghi.\n", full_path);
                    continue;
                }
                // Nhận và ghi nội dung của file từ client
                printf("Receiving image....\n");
                int receiving_image  = 0;
                while (1)
                {
                    // char request[MAX_MESSAGE_SIZE * 2];
                    ssize_t bytes_received = recv(client_socket, request, sizeof(request), 0);
                    if (bytes_received <= 0)
                    {
                        printf("Receive empty\n");
                        break;
                    }
                    else if (strstr(request, "IMAGE_CONTENT") != NULL)
                    {
                        receiving_image = 1;
                        printf("Receiving image data...\n");
                    }
                    else if (receiving_image)
                    {
                        printf("Writing into File\n");
                        fwrite(request, 1, bytes_received, file);
                        if (strstr(request, "END") != NULL) {
                            printf("Image received and saved.\n");
                            receiving_image = 0;
                            fclose(file); // Đóng tập tin khi hình ảnh được nhận và lưu hoàn tất
                            break;
                        }
                    }
                    
                }
                char okMessage[] = "OK";
                int okSent = send(client_socket, okMessage, strlen(okMessage), 0);
                if (okSent == -1)
                {
                    perror("send");
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
