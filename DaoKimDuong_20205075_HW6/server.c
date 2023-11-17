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
                snprintf(full_path, sizeof(full_path), "%s/%s", directory, filename);
                FILE *file = fopen(full_path, "wb");

                if (file == NULL)
                {
                    printf("Không thể mở tệp %s để ghi.\n", full_path);
                    continue;
                }
                int receiving_image = 0;
                char buffer[MAX_MESSAGE_SIZE];
                size_t buffer_length = 0;
                char remaining_end_marker[MAX_MESSAGE_SIZE]; // Biến để lưu trữ phần "IMAGE_CONTENTEND" không nằm trong buffer
                size_t remaining_end_marker_length = 0;      // Độ dài của phần "IMAGE_CONTENTEND" không nằm trong buffer
                while (1)
                {
                    char image_data[MAX_MESSAGE_SIZE];
                    size_t bytes_received = recv(client_socket, image_data, sizeof(image_data), 0);

                    if (bytes_received <= 0)
                    {
                        printf("Receive empty\n");
                        break;
                    }

                    if (receiving_image)
                    {
                        if (strstr(image_data, "IMAGE_CONTENTEND") != NULL) {
                            printf("lasst\n");
                        }
                            
                        size_t end_marker_length = strlen("IMAGE_CONTENTEND");
                        if (buffer_length >= end_marker_length)
                        {
                            // Di chuyển phần cuối của "IMAGE_CONTENTEND" từ buffer vào biến remaining_end_marker
                            memcpy(remaining_end_marker, buffer + buffer_length - end_marker_length, end_marker_length);
                            remaining_end_marker_length = end_marker_length;

                            // Giảm buffer_length đi độ dài của phần cuối của "IMAGE_CONTENTEND"
                            buffer_length -= end_marker_length;
                        }
                        fwrite(image_data, 1, bytes_received, file);
                        printf("bytes received %zu\n", bytes_received);
                        buffer_length = 0; // Đặt buffer_length về 0 vì toàn bộ dữ liệu đã được ghi vào tệp
                                           
                        // Kiểm tra sự xuất hiện của "IMAGE_CONTENTEND" trong dữ liệu mới
                        for (size_t i = 0; i < bytes_received; i++)
                        {
                            if (remaining_end_marker_length < end_marker_length)
                            {
                                // Lưu từng ký tự vào remaining_end_marker cho đến khi có đủ "IMAGE_CONTENTEND"
                                remaining_end_marker[remaining_end_marker_length] = image_data[i];
                                remaining_end_marker_length++;
                            }
                            else
                            {
                                // So sánh phần cuối của remaining_end_marker với "IMAGE_CONTENTEND"
                                if (memcmp(remaining_end_marker + remaining_end_marker_length - end_marker_length, "IMAGE_CONTENTEND", end_marker_length) == 0)
                                {
                                    // Gặp "IMAGE_CONTENTEND," kết thúc quá trình nhận hình ảnh
                                    printf("Image received and saved.\n");
                                    receiving_image = 0;
                                    // Ghi dữ liệu từ remaining_end_marker trừ phần "IMAGE_CONTENTEND"
                                    fwrite(remaining_end_marker, 1, remaining_end_marker_length - end_marker_length, file);
                                    remaining_end_marker_length = 0;
                                    fclose(file);
                                    break;
                                }
                            }
                        }
                        if (memcmp(image_data, "IMAGE_CONTENTEND", strlen("IMAGE_CONTENTEND")) == 0)
                        {
                            printf("Image received and saved.\n");
                            receiving_image = 0;
                            fclose(file);
                            break;
                        }
                    }
                    else
                    {
                        // Nếu chưa bắt đầu nhận hình ảnh, kiểm tra xem có chuỗi "IMAGE_CONTENT" trong dữ liệu không.
                        if (bytes_received >= strlen("IMAGE_CONTENT") && memcmp(image_data, "IMAGE_CONTENT", strlen("IMAGE_CONTENT")) == 0)
                        {
                            receiving_image = 1;

                            // Bỏ qua chuỗi "IMAGE_CONTENT" khi ghi vào tệp nếu đã thấy nó.
                            int data_start = strlen("IMAGE_CONTENT");
                            int data_length = bytes_received - data_start;
                            if (data_length > 0)
                            {
                                fwrite(image_data + data_start, 1, data_length, file);
                                printf("bytes recived %zu\n", bytes_received);
                            }
                        }
                        else
                        {
                            // Lưu dữ liệu vào buffer cho đến khi gặp "IMAGE_CONTENT"
                            if (buffer_length + bytes_received <= MAX_MESSAGE_SIZE)
                            {
                                memcpy(buffer + buffer_length, image_data, bytes_received);
                                buffer_length += bytes_received;
                            }
                            else
                            {
                                printf("Buffer overflow.\n");
                                // Xử lý trường hợp tràn buffer nếu cần
                            }
                        }
                    }
                }
                printf("DONE\n");
                char okMessage[] = "OK";
                int okSent = send(client_socket, okMessage, strlen(okMessage), 0);
                if (okSent == -1)
                {
                    perror("send");
                }
                break;
            }
        }
    }

    close(server_socket);
    return 0;
}
