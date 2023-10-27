/*
    Author: Dao Kim Duong - 20205075, SoICT, HUST
    Problem:

Create a network application using UDP sockets with the following requirements, using the account.txt file from HW1.
Server-side:
- Run on any port number specified as a command-line parameter, for example:
  `./server PortNumber` (e.g., `./server 5500`).
- Receive requests from clients containing a username and password (only one login per request).
- If the password is correct:
  - If the account is active, send the result "OK."
  - If the account is blocked or not activated: send "Account not ready."
- If the password is incorrect, send "Not OK."
  - After three failed attempts, block the account.
- After successful login:
  - Accept password change requests from the client (any string other than "bye").
  - If the new password contains characters other than numbers or alphabets, report an error.
  - Return the new password to the client, securely hashed using SHA256, as a string containing both alphabetic and numeric characters. If the string contains characters other than alphabets or numbers, send an error message.
  - Accept sign-out requests by receiving the "bye" string from the client.

Client-side:
- Connect to the server using the IP address and port number specified as command-line parameters. Syntax:
  `./client IPAddress PortNumber` (e.g., `./client 127.0.0.1 5500`).
- Allow users to input the account, password, and new password from the keyboard and send them to the server.
- Receive and display responses from the server.
- Provide a looping function until the user enters an empty string.

Note: Create a Makefile with the executable file names "server" and "client" after compilation.

Example file content:
```
hust hust123 1
soict soict123 0
```
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

typedef struct
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int isActive;      // 0: Tài khoản bị khoá, 1: Tài khoản hoạt động
    int loginAttempts; // Số lần đăng nhập thất bại
    int isLoggedIn;    // Trạng thái đăng nhập, 0: Đăng xuất, 1: Đã đăng nhập
} User;

User users[MAX_USERS];
int userCount = 0;

// Hàm để kiểm tra xem tài khoản có tồn tại hay không
User *findUser(const char *username)
{
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            return &users[i];
        }
    }
    return NULL; // Không tìm thấy tài khoản
}

// Hàm để kiểm tra tính hợp lệ của mật khẩu
int isPasswordValid(User *user, const char *password)
{
    if (strcmp(user->password, password) == 0)
    {
        return 1; // Mật khẩu hợp lệ
    }
    return 0; // Mật khẩu không hợp lệ
}

void loadUser()
{
    // Đọc danh sách người dùng từ tệp
    FILE *file = fopen("account.txt", "r");
    if (file != NULL)
    {
        while (userCount < MAX_USERS && fscanf(file, "%s %s %d", users[userCount].username, users[userCount].password, &users[userCount].isActive) == 3)
        {
            users[userCount].loginAttempts = 0;
            users[userCount].isLoggedIn = 0; // Đảm bảo tất cả người dùng là đăng xuất ban đầu
            userCount++;
        }
        fclose(file);
    }
    else
    {
        printf("Lỗi đọc tệp account.txt\n");
    }
}

void printUserList(User users[], int userCount)
{
    printf("Danh sách người dùng:\n");
    for (int i = 0; i < userCount; i++)
    {
        printf("Username: %s, Password: %s, isActive: %d, Login Attempts: %d, Is Logged In: %d\n", users[i].username, users[i].password, users[i].isActive, users[i].loginAttempts, users[i].isLoggedIn);
    }
}
void sha256(const char *input, char outputBuffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(hash, &sha256);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = '\0';
}
// Hàm kiểm tra tính hợp lệ của mật khẩu mới
int isValidNewPassword(const char *newPassword)
{
    for (int i = 0; newPassword[i] != '\0'; i++)
    {
        if (!isalnum(newPassword[i]))
        {
            return 0; // Mật khẩu không hợp lệ
        }
    }
    return 1; // Mật khẩu hợp lệ
}
void processChangePasswordRequest(const char *clientRequest, char *response)
{
    char username[MAX_USERNAME_LENGTH];
    char newPassword[MAX_PASSWORD_LENGTH];
    sscanf(clientRequest, "CHANGEPW %s %s", username, newPassword);

    User *user = findUser(username);

    if (user == NULL)
    {
        // Trường hợp không có username trong hệ thống
        strcpy(response, "Not existed");
    }
    else
    {
        if (isValidNewPassword(newPassword)) {
            // Cập nhật mật khẩu mới cho tài khoản
            strcpy(user->password, newPassword);
            // Mã hóa mật khẩu mới bằng SHA-256
            char hashedPassword[65];
            sha256(newPassword, hashedPassword);

            // Tạo hai chuỗi riêng lẻ cho chữ và chữ số
            char alphaChars[65];
            char digitChars[65];

            int alphaIndex = 0;
            int digitIndex = 0;

            for (int i = 0; i < 64; i++)
            {
                if (isalpha(hashedPassword[i]))
                {
                    alphaChars[alphaIndex++] = hashedPassword[i];
                }
                else if (isdigit(hashedPassword[i]))
                {
                    digitChars[digitIndex++] = hashedPassword[i];
                }
            }

            alphaChars[alphaIndex] = '\0';
            digitChars[digitIndex] = '\0';

            // Trả về kết quả theo định dạng "HASHED %s %s"
            sprintf(response, "HASHED %s %s", alphaChars, digitChars);
        }
        else
        {
            // Trường hợp mật khẩu mới không hợp lệ
            strcpy(response, "Invalid password");
        }
    }
}

// Hàm để xử lý yêu cầu từ client
void processRequest(User users[], int userCount, const char *clientRequest, char *response)
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    
    if (strncmp(clientRequest, "LOGIN", 5) == 0) {
        sscanf(clientRequest, "LOGIN %s %s", username, password);
        User *user = findUser(username);

        if (user == NULL)
        {
            // Trường hợp không có username trong hệ thống
            strcpy(response, "Not existed");
        }
        else
        {
            if (strcmp(user->password, password) == 0)
            {
                // Trường hợp đúng username và password
                if (user->isActive == 1)
                {
                    // Trường hợp tài khoản hoạt động
                    strcpy(response, "OK");
                    user->loginAttempts = 0;
                    user->isLoggedIn = 1; // Đánh dấu tài khoản đã đăng nhập
                }
                else
                {
                    // Trường hợp tài khoản không hoạt động
                    strcpy(response, "Account not ready");
                }
            }
            else
            {
                // Trường hợp đúng username nhưng sai password
                strcpy(response, "Not OK");
                user->loginAttempts++;
                if (user->loginAttempts >= 3)
                {
                    user->isActive = 0; // Khoá tài khoản nếu nhập sai quá 3 lần
                    strcpy(response, "Account is blocked!");
                }
            }
        }
    }
    else if (strncmp(clientRequest, "CHANGEPW", 8) == 0) {
        sscanf(clientRequest, "CHANGEPW %s %s", username, password);
        processChangePasswordRequest(clientRequest, response);
    }
    else if (strncmp(clientRequest, "LOGOUT", 6) == 0)
    {
        sscanf(clientRequest, "LOGOUT %s", username);
        // Đăng xuất người dùng
        User *user = findUser(username);
        if (user != NULL)
        {
            user->isLoggedIn = 0; // Đánh dấu tài khoản đã đăng xuất
            sprintf(response, "Bye %s", username);
        }
        else
        {
            strcpy(response, "You are not logged in.");
        }
    }

    printUserList(users, userCount);
}

int main(int argc, char *argv[])
{
    loadUser();
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <PortNumber>\n", argv[0]);
        exit(1);
    }
    int portNumber = atoi(argv[1]);

    // Tạo socket
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1)
    {
        perror("Không thể tạo socket");
        exit(1);
    }
    // Cấu hình địa chỉ và cổng cho server
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // Ràng buộc socket với địa chỉ và cổng
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Không thể ràng buộc socket");
        close(serverSocket);
        exit(1);
    }

    while (1)
    {
        char clientRequest[100];
        char response[50];
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        // Nhận yêu cầu từ client
        ssize_t bytesReceived = recvfrom(serverSocket, clientRequest, sizeof(clientRequest), 0, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (bytesReceived == -1)
        {
            perror("Lỗi khi nhận dữ liệu từ client");
            continue;
        }

        // Xử lý yêu cầu từ client và tạo phản hồi
        processRequest(users, userCount, clientRequest, response);

        // Gửi phản hồi về cho client
        if (sendto(serverSocket, response, strlen(response), 0, (struct sockaddr *)&clientAddress, clientAddressLength) == -1)
        {
            perror("Lỗi khi gửi phản hồi cho client");
        }

        memset(clientRequest, 0, sizeof(clientRequest));
        memset(response, 0, sizeof(response));
    }

    // Đóng socket và kết thúc chương trình
    close(serverSocket);

    return 0;
}
