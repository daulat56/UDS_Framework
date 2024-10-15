# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -Wextra -g

# Server and Client Executable Names
SERVER_TARGET = server_o
CLIENT_TARGET = client_o

# Source Files
SERVER_SRCS = server.c services.c
CLIENT_SRCS = client.c

# Object Files
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

# Header Files
DEPS = services.h server.h

# Rule to build both server and client
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Rule to build the server executable
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

# Rule to build the client executable
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS)

# Rule to build object files for server
$(SERVER_OBJS): %.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build object files for client
$(CLIENT_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean the build
.PHONY: clean
clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_TARGET) $(CLIENT_TARGET)
