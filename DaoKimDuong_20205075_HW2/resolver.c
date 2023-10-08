#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

const char* param;
struct hostent* host_entry;
struct in_addr addr;

// Hàm in ra tên miền chính thức và tên miền định danh của máy chủ
void printOfficialDomainAndAliases() {
    printf("Official name: %s\n", host_entry->h_name);

    char **aliases;
    printf("Alias name:\n");
    for (aliases = host_entry->h_aliases; *aliases != NULL; aliases++) {
        printf("%s\n", *aliases);
    }
}

// Hàm in ra địa chỉ IP chính thức và các địa chỉ IP định danh của máy chủ
void printOfficialIPAndAliases() {
    printf("Official IP:\n");
    char **ip;
    for (ip = host_entry->h_addr_list; *ip != NULL; ip++) {
        struct in_addr addr;
        memcpy(&addr.s_addr, *ip, sizeof(addr.s_addr));
        printf("%s\n", inet_ntoa(addr));
    }
}

// Hàm xử lý chính của chương trình
void solve() {
    // Thử chuyển đổi param thành địa chỉ IP
    if (inet_aton(param, &addr) != 0)
    {
        host_entry = gethostbyaddr(&addr, sizeof(addr), AF_INET);
        if (host_entry != NULL)
        {
            // Lấy tên miền chính thức và tên miền định danh
            printOfficialDomainAndAliases();
        } else
        {
            printf("Not found information\n");
        }
    } else
    {
        host_entry = gethostbyname(param);
        if (host_entry != NULL) 
        {
           // Lấy địa chỉ IP chính thức và các địa chỉ IP định danh
           printOfficialIPAndAliases();
        } else {
            printf("Not found information\n");
        }
        
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Please enter a domain name or IP address\n");
        exit(1);
    }
    param = argv[1];
    solve();
    return 0;
}