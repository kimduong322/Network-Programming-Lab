#include <stdio.h>
#include<stdlib.h>
#include<string.h>
struct User
{
    char username[50];
    char password[50];
    int status;
    struct User * next;
};

struct LoginAttempts {
    char username[50];
    int attemps;
};
struct LoggedInUser
{
    char username[50];
    struct LoggedInUser* next;
};
// Số lượng tài khoản tối đa để xác định kích thước của mảng
const int MAX_USERS = 100;

// Mảng lưu trữ số lần đăng nhập sai cho mỗi người dùng
struct LoginAttempts loginAttemptsArray[100];

int isLogined = 0;

// Hàm để thêm một nút mới vào danh sách liên kết
struct User* addNode(struct User* head, const char* username, const char* password, int status) {
    struct User* newUser = (struct User*)malloc(sizeof(struct User));
    if (newUser == NULL) {
        printf("Memory allocation failed.\n");
        return head;
    }

    strcpy(newUser->username, username);
    strcpy(newUser->password, password);
    newUser->status = status;
    newUser->next = head;
    head = newUser;
    return head;
}

// Hàm để in danh sách liên kết
void printList(struct User* head) {
    struct User* current = head;
    printf("List of Users:\n");
    while (current != NULL) {
        printf("Username: %s, Password: %s, Status: %d\n", current->username, current->password, current->status);
        current = current->next;
    }
}
struct User* searchUser(struct User* head, const char* usernameToFind)
{
    struct User* current = head;

    while (current != NULL)
    {
        if (strcmp(current->username, usernameToFind) == 0)
        {
            // Tìm thấy người dùng có tên người dùng tương ứng
            return current;
        }
        current = current->next;
    }

    // Không tìm thấy người dùng có tên người dùng tương ứng
    return NULL;
}
void regist(struct User** head, const char* username, const char* password) {
    struct User* foundUser = searchUser(*head, username);
    if (foundUser != NULL)
    {
        printf("Account existed\n");
    } else
    {
        *head = addNode(*head, username, password, 1);
        // Mở tệp output.txt để cập nhật thông tin tài khoản mới
        FILE* outputFile = fopen("account.txt", "a");
        if (outputFile == NULL) {
            printf("Failed to open output.txt for writing.\n");
            return;
        }
        // Ghi thông tin tài khoản mới vào tệp
        fprintf(outputFile, "%s %s %d\n", username, password, 1);

        // Đóng tệp
        fclose(outputFile);

        printf("Successful registration\n");
    } 
}

// Hàm kiểm tra xem một tài khoản có bị chặn (blocked) hay không
int notBlock(struct User* head, const char* username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(loginAttemptsArray[i].username, username) == 0) {
            // Tài khoản đã tồn tại trong mảng
            struct User* current = searchUser(head, username);
            if (current->status == 0) return 0;
            
            else if (loginAttemptsArray[i].attemps > 3) {
                // Nếu số lần đăng nhập sai lớn hơn hoặc bằng 3, tài khoản bị chặn
                return 0; // Trả về 0 để chỉ ra tài khoản đã bị chặn
            } else {
                return 1; // Trả về 1 để chỉ ra tài khoản chưa bị chặn
            }
        }
    }
    return 1; // Trả về 1 để chỉ ra tài khoản chưa bị chặn
}

// Hàm tăng số lần đăng nhập sai cho một tài khoản
void increaseNumbofIncorrectPassword(const char* username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(loginAttemptsArray[i].username, username) == 0) {
            // Tài khoản đã tồn tại trong mảng
            loginAttemptsArray[i].attemps++;
            break;
        }
    }
}

int isCorrectUserName(struct User* head, const char* userName) {
    return searchUser(head, userName) != NULL;
}

int isCorrectPassword(struct User* userList, const char* username, const char* password) {
    struct User* user = searchUser(userList, username);

    if (user != NULL && strcmp(user->password, password) == 0) {
        // Tên người dùng tồn tại và mật khẩu khớp
        return 1; // Trả về 1 để chỉ ra mật khẩu chính xác
    } else {
        return 0; // Trả về 0 để chỉ ra mật khẩu không chính xác hoặc tên người dùng không tồn tại
    }
}
// Hàm để cập nhật trạng thái của người dùng trong danh sách liên kết
void updateStatus(struct User* userList, const char* username, int newStatus) {
    struct User* current = userList;

    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            // Tìm thấy người dùng có tên người dùng tương ứng
            current->status = newStatus;
            break;
        }
        current = current->next;
    }
}

void saveUserListToFile(struct User* userList) {
    FILE* file = fopen("account.txt", "w");
    if (file == NULL) {
        printf("Failed to open file.\n");
        return;
    }

    struct User* current = userList;

    while (current != NULL) {
        fprintf(file, "%s %s %d\n", current->username, current->password, current->status);
        current = current->next;
    }

    fclose(file);
}
// Hàm để thêm một tên người dùng đã đăng nhập thành công vào danh sách
struct LoggedInUser* addLoggedInUser(struct LoggedInUser* loggedInUsers, const char* username) {
    struct LoggedInUser* newUser = (struct LoggedInUser*)malloc(sizeof(struct LoggedInUser));
    if (newUser == NULL) {
        printf("Memory allocation failed.\n");
        return loggedInUsers;
    }

    strcpy(newUser->username, username);
    newUser->next = loggedInUsers;
    return newUser;
}

// Hàm để kiểm tra xem một tên người dùng đã đăng nhập thành công hay chưa
int isUserLoggedIn(struct LoggedInUser* loggedInUsers, const char* username) {
    struct LoggedInUser* current = loggedInUsers;

    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            // Tên người dùng đã đăng nhập thành công
            return 1;
        }
        current = current->next;
    }

    // Tên người dùng chưa đăng nhập thành công
    return 0;
}

// Hàm để xóa một tên người dùng đã đăng nhập thành công khỏi danh sách
struct LoggedInUser* removeLoggedInUser(struct LoggedInUser* loggedInUsers, const char* username) {
    struct LoggedInUser* current = loggedInUsers;
    struct LoggedInUser* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            // Tìm thấy tên người dùng trong danh sách, xóa nó ra khỏi danh sách
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                loggedInUsers = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }

    return loggedInUsers;
}

// Hàm để hiển thị tất cả các người dùng đã đăng nhập
void showLoggedInUsers(struct LoggedInUser* loggedInUsers) {
    printf("Logged In Users:\n");
    struct LoggedInUser* current = loggedInUsers;

    while (current != NULL) {
        printf("%s\n", current->username);
        current = current->next;
    }
}

int main() {
    int choice;
    char line[150]; // Dùng để đọc mỗi dòng từ file
    char username[50];
    char password[50];
    int status;
    char username1[50];
    struct User* userList = NULL;
    // Khởi tạo danh sách các tên người dùng đã đăng nhập thành công
    struct LoggedInUser* loggedInUsers = NULL;
    // load user from file account.txt and store to linked-list to
    FILE* file = fopen("account.txt", "r");
    if (file == NULL) {
        printf("Cannot open file account.txt.\n");
        return 1;
    }
    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        // Sử dụng sscanf để phân tách dòng thành các trường
        if (sscanf(line, "\n%s %s %d", username, password, &status) == 3) {
            userList = addNode(userList, username, password, status);
            strcpy(loginAttemptsArray[i].username, username);
            loginAttemptsArray[i].attemps = 0; 
            i++;
        }
    }
    fclose(file);

    // In danh sách người dùng
    // printList(userList);

    while (1) {
        // Hiển thị menu cho người dùng
        printf("USER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Search\n");
        printf("4. Sign out\n");
        printf("Your choice (1-4, other to quit): ");
        scanf("%d", &choice);

        // Xử lý lựa chọn của người dùng
        switch (choice) {
            case 1:
                printf("Register.\nUsername = ");
                scanf("%s", username);
                printf("Password = ");
                scanf("%s", password);
                regist(&userList, username, password);
                // printList(userList);
                break;
            case 2:

                printf("Sign in.\nUsername = ");
                scanf("%s", username);
                if (isCorrectUserName(userList, username)) {
                    if (notBlock(userList, username)) {
                        printf("Password = ");
                        scanf("%s", password);
                        if (isCorrectPassword(userList, username, password)) {
                            printf("Hello %s\n", username);
                            loggedInUsers = addLoggedInUser(loggedInUsers, username);
                            showLoggedInUsers(loggedInUsers);
                        } else {
                            increaseNumbofIncorrectPassword(username);
                            printf("Password is incorrect\n");
                        }
                    } else {
                        printf("Account is blocked\n");
                        updateStatus(userList, username, 0);
                        saveUserListToFile(userList);
                    }
                    
                } else printf("Cannot find account\n");
                break;
            case 3:
                printf("Search.\n");
                printf("Username = ");
                scanf("%s", username);
                struct User* user = searchUser(userList, username);
                if (user != NULL) {
                    if (user->status == 0) printf("Account is blocked\n");
                    else printf("Account is active\n");
                } else printf("Cannot find account\n");
                break;
            case 4:
                printf("Username = ");
                scanf("%s", username1);
                struct User* user1 = searchUser(userList, username1);
                if (user1 != NULL) {
                    if (isUserLoggedIn(loggedInUsers, username1) == 1)
                    {
                        printf("Good bye %s\n", username1);
                        loggedInUsers = removeLoggedInUser(loggedInUsers, username1);
                    } else printf("Account is not sign in\n");
                    

                } else printf("Cannot find account\n");
                break;
            default:
                return -1;
        }
    }

    return 0;
}
