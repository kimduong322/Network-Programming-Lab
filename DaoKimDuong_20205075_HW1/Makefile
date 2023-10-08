CC = gcc
CFLAGS = -Wall

# Tên tệp đầu ra
OUTPUT = main.exe

# Danh sách tệp nguồn
SOURCES = main.c

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCES)

clean:
	rm -f $(OUTPUT)
