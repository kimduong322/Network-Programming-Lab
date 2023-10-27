CC = gcc
CFLAGS = -Wall -Wextra

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server -lssl -lcrypto

client: client.c
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f server client

help:
	@echo "Các lựa chọn cho lệnh make:"
	@echo "  - make all: Biên dịch cả server và client."
	@echo "  - make server: Biên dịch server."
	@echo "  - make client: Biên dịch client."
	@echo "  - make clean: Xóa các tệp đã biên dịch."
	@echo "  - make help: Hiển thị hướng dẫn này."

.PHONY: all server client clean help
