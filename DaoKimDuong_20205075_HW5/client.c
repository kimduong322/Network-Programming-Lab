/*
    Author: Dao Kim Duong - 20205075, SoICT, HUST
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        exit(1);
    }

    const char *serverIP = argv[1];
    int portNumber = atoi(argv[2]);

    // Tạo socket
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1)
    {
        perror("Không thể tạo socket");
        exit(1);
    }

    // Cấu hình địa chỉ và cổng cho server
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = inet_addr(serverIP);

    // Nhập username và password
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    printf("Nhập username: ");
    scanf("%s", username);
    printf("Nhập password: ");
    scanf("%s", password);

    // Gửi yêu cầu đăng nhập đến server
    char request[200];
    sprintf(request, "LOGIN %s %s", username, password);

    if (sendto(clientSocket, request, strlen(request), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Lỗi khi gửi yêu cầu đăng nhập");
        close(clientSocket);
        exit(1);
    }

    // Nhận phản hồi từ server
    char response[50];
    struct sockaddr_in serverResponseAddress;
    socklen_t serverResponseAddressLength = sizeof(serverResponseAddress);
    ssize_t bytesReceived = recvfrom(clientSocket, response, sizeof(response), 0, (struct sockaddr *)&serverResponseAddress, &serverResponseAddressLength);

    if (bytesReceived == -1)
    {
        perror("Lỗi khi nhận phản hồi từ server");
        close(clientSocket);
        exit(1);
    }

    response[bytesReceived] = '\0';

    printf("Phản hồi từ server: %s\n", response);

    if (strcmp(response, "OK") == 0)
    {
        // Đăng nhập thành công
        char newPassword[MAX_PASSWORD_LENGTH];
        while (1)
        {
            printf("Nhập mật khẩu mới (hoặc 'bye' để đăng xuất): ");
            scanf("%s", newPassword);

            if (strcmp(newPassword, "bye") == 0)
            {
                char logout[120];
                char response[120];
                sprintf(logout, "LOGOUT %s", username);
                // Đăng xuất
                sendto(clientSocket, logout, strlen(logout), 0, (struct sockaddr *)&serverResponseAddress, serverResponseAddressLength);
                bytesReceived = recvfrom(clientSocket, response, sizeof(response), 0, (struct sockaddr *)&serverResponseAddress, &serverResponseAddressLength);
                response[bytesReceived] = '\0';
                printf("Phản hồi từ server sau đổi mật khẩu: %s\n", response);
                break;
            }
            else
            {
                // Gửi yêu cầu đổi mật khẩu
                char changePasswordRequest[120];
                sprintf(changePasswordRequest, "CHANGEPW %s %s", username, newPassword);
                sendto(clientSocket, changePasswordRequest, strlen(changePasswordRequest), 0, (struct sockaddr *)&serverResponseAddress, serverResponseAddressLength);

                // Nhận phản hồi về việc đổi mật khẩu
                bytesReceived = recvfrom(clientSocket, response, sizeof(response), 0, (struct sockaddr *)&serverResponseAddress, &serverResponseAddressLength);
                if (bytesReceived == -1)
                {
                    perror("Lỗi khi nhận phản hồi đổi mật khẩu từ server");
                    break;
                }
                response[bytesReceived] = '\0';
                if (strncmp(response, "HASHED", 6) == 0)
                {
                    char alphaChars[65];
                    char digitChars[65];

                    // Sử dụng sscanf để trích xuất hai chuỗi từ response
                    if (sscanf(response, "HASHED %s %s", alphaChars, digitChars) == 2)
                    {
                        printf("Chuỗi chứa ký tự chữ (alphaChars): %s\n", alphaChars);
                        printf("Chuỗi chứa ký tự số (digitChars): %s\n", digitChars);
                    }
                    else
                    {
                        printf("Lỗi trong phản hồi từ server.\n");
                    }
                }
                else if(strcmp(response, "Invalid password") == 0)
                    {
                        printf("Lỗi: Mật khẩu mới không hợp lệ.\n");
                    }
                else
                {
                    printf("Phản hồi từ server: %s\n", response);
                }
            }
        }
    }

    close(clientSocket);

    return 0;
}
