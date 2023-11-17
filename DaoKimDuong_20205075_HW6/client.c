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
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        exit(1);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket == -1)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error! Cannot connect to server. Client exits immediately!");
        exit(1);
    }

    int choice;
    char input[MAX_MESSAGE_SIZE];
    char alpha[MAX_MESSAGE_SIZE];

    printf("MENU\n");
    printf("-----------------------------------\n");
    printf("1. Gửi xâu bất kỳ\n");
    printf("2. Gửi nội dung một file\n");

    while (1)
    {
        printf("Chọn chế độ (1 hoặc 2): ");
        scanf("%d", &choice);

        if (choice == 1)
        {
            getchar();
            while (1)
            {
                printf("Nhập xâu (Kết thúc bằng Enter): ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;
                if (strlen(input) == 0)
                {
                    printf("Exited\n");
                    break;
                }
                char request[MAX_MESSAGE_SIZE * 2];
                sprintf(request, "STRING %s", input);
                send(client_socket, request, strlen(request), 0);

                recv(client_socket, alpha, sizeof(alpha), 0);

                if (strncmp(alpha, "ERROR:", 6) == 0)
                {
                    printf("Server returned: %s\n", alpha);
                }
                else if (strncmp(alpha, "OK", 2) == 0)
                {
                    printf("Server returned: OK\n");
                }
            }
        }
        else if (choice == 2)
        {
            char filename[MAX_MESSAGE_SIZE];
            printf("Input file stored in ClientImgSrc folder (Ex: question.png): ");
            scanf("%s", filename);

            const char *directory = "ClientImgSrc";
            char file_path[MAX_MESSAGE_SIZE + 1];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory, filename);
            FILE *file = fopen(file_path, "rb");

            if (file == NULL)
            {
                printf("Cannot open file %s.\n", file_path);
                break;
            }

            // Gửi tên và kiểu file cho server
            char file_type[MAX_MESSAGE_SIZE];
            char *dot = strrchr(filename, '.');
            if (dot && dot != filename)
            {
                strcpy(file_type, dot + 1);
            }
            char request[MAX_MESSAGE_SIZE * 3];
            sprintf(request, "IMAGE_NAME_AND_TYPE %s %s", filename, file_type);
            send(client_socket, request, strlen(request), 0);

            printf("Sending image...\n");
            char begin_request[MAX_MESSAGE_SIZE];
            sprintf(begin_request, "IMAGE_CONTENT");
            send(client_socket, begin_request, strlen(begin_request), 0);
            printf("send IMAGE_CONTENT\n");
            size_t bytes_read;
            char image_data[MAX_MESSAGE_SIZE];
            
            while ((bytes_read = fread(image_data, 1, sizeof(image_data), file)) > 0)
            {
                
                if (bytes_read < MAX_MESSAGE_SIZE)
                {
                    char partial_data[(int)bytes_read];
                    memcpy(partial_data, image_data, bytes_read);
                    send(client_socket, partial_data, bytes_read, 0);
                    printf("send %zu bytes:\n", bytes_read);
                    for (size_t i = 0; i < bytes_read; i++)
                    {
                        printf("%02X ", (unsigned char)partial_data[i]);
                    }
                    printf("\n");
                } else {
                    send(client_socket, image_data, bytes_read, 0);
                    for (size_t i = 0; i < bytes_read; i++)
                    {
                        printf("%02X ", (unsigned char)image_data[i]);
                    }
                    printf("\n");
                }
                
                printf("send %zu bytes\n", bytes_read);
            }

            // Đóng file
            fclose(file);

            // Gửi thông điệp kết thúc sau khi gửi toàn bộ dữ liệu hình ảnh
            // char end_request[MAX_MESSAGE_SIZE]; // Đủ dài để chứa "IMAGE_CONTENTEND"
            // sprintf(end_request, "IMAGE_CONTENTEND");
            // send(client_socket, end_request, strlen(end_request), 0);
            // send(client_socket, "IMAGE_CONTENTEND", strlen("IMAGE_CONTENTEND"), 0);
            // Độ dài của chuỗi "IMAGE_CONTENTEND" (bao gồm cả null terminator '\0')
            size_t end_marker_length = strlen("IMAGE_CONTENTEND");

            // Tạo mảng để lưu trữ dữ liệu binary của "IMAGE_CONTENTEND"
            char image_content_end_data[end_marker_length];

            // Sao chép dữ liệu binary từ chuỗi "IMAGE_CONTENTEND" vào mảng
            for (size_t i = 0; i < end_marker_length; i++)
            {
                image_content_end_data[i] = "IMAGE_CONTENTEND"[i];
            }
            send(client_socket, image_content_end_data, sizeof(image_content_end_data), 0);

            printf("DONE\n");

            // Nhận phản hồi từ server
            recv(client_socket, alpha, sizeof(alpha), 0);
            if (strncmp(alpha, "OK", 2) == 0)
            {
                printf("Server returned: OK\n");
            }
            else
            {
                printf("Server returned an error: %s\n", alpha);
            }
        }
    }

    return 0;
}
