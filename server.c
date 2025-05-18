#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_TODO 10
#define BUFFER_SIZE 1024

typedef struct {
    char task[256];
    int completed;
} Todo;

Todo todo_list[MAX_TODO];
int todo_count = 0;

void print_usage() {
    printf("Server is running on port %d\n", PORT);
    printf("\n********** TODO API Server **********n");
    printf("1. Add a Task: \n");
    printf("   curl -X POST -d \"Your task description\" http://localhost:%d/todos\n", PORT);
    printf("\n2. View All Tasks: \n");
    printf("   curl http://localhost:%d/todos\n", PORT);
    printf("\n3. Complete a Task (by index): \n");
    printf("   curl -X PUT http://localhost:%d/todos/<index>/complete\n", PORT);
    printf("   Example: curl -X PUT http://localhost:%d/todos/1/complete\n", PORT);
    printf("\n4. Delete a Task (by index): \n");
    printf("   curl -X DELETE http://localhost:%d/todos/<index>\n", PORT);
    printf("   Example: curl -X DELETE http://localhost:%d/todos/1\n", PORT);
    printf("\n5. View Task History (Completed Tasks): \n");
    printf("   curl http://localhost:%d/todos/history\n", PORT);
    printf("********************************************\n\n");
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
    buffer[bytes_read] = '\0';

    if (strncmp(buffer, "GET /todos/history", 18) == 0) {
        char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\nContent: text/plain\n\n";
        for (int i = 0; i < todo_count; i++) {
            if (todo_list[i].completed) {
                char todo_item[300];
                snprintf(todo_item, sizeof(todo_item), "%d. %s [Completed]\n", i + 1, todo_list[i].task);
                strcat(response, todo_item);
            }
        }
        write(client_socket, response, strlen(response));
    } 
    else if (strncmp(buffer, "GET /todos", 10) == 0) {
        char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\nContent: text/plain\n\n";
        for (int i = 0; i < todo_count; i++) {
            char todo_item[300];
            snprintf(todo_item, sizeof(todo_item), "%d. %s [%s]\n", i + 1, todo_list[i].task, todo_list[i].completed ? "Completed" : "Pending");
            strcat(response, todo_item);
        }
        write(client_socket, response, strlen(response));
    } 
    else if (strncmp(buffer, "POST /todos", 11) == 0) {
        char *task_start = strstr(buffer, "\r\n\r\n");
        if (task_start && todo_count < MAX_TODO) {
            task_start += 4;
            strncpy(todo_list[todo_count].task, task_start, 255);
            todo_list[todo_count].completed = 0;
            todo_count++;
            char response[] = "HTTP/1.1 201 Created\nContent: text/plain\n\nTask added.\n";
            write(client_socket, response, strlen(response));
        } else {
            char response[] = "HTTP/1.1 400 Bad Request\nContent: text/plain\n\nInvalid request or list full.\n";
            write(client_socket, response, strlen(response));
        }
    } 
    else if (strncmp(buffer, "PUT /todos/", 11) == 0 && strstr(buffer, "/complete")) {
        int index = atoi(buffer + 11) - 1;
        if (index >= 0 && index < todo_count) {
            todo_list[index].completed = 1;
            char response[] = "HTTP/1.1 200 OK\nContent: text/plain\n\nTask marked as completed.\n";
            write(client_socket, response, strlen(response));
        } else {
            char response[] = "HTTP/1.1 404 Not Found\nContent: text/plain\n\nTask not found.\n";
            write(client_socket, response, strlen(response));
        }
    } 
    else if (strncmp(buffer, "DELETE /todos/", 14) == 0) {
        int index = atoi(buffer + 14) - 1;
        if (index >= 0 && index < todo_count) {
            for (int i = index; i < todo_count - 1; i++) {
                todo_list[i] = todo_list[i + 1];
            }
            todo_count--;
            char response[] = "HTTP/1.1 200 OK\nContent: text/plain\n\nTask deleted.\n";
            write(client_socket, response, strlen(response));
        } else {
            char response[] = "HTTP/1.1 404 Not Found\nContent: text/plain\n\nTask not found.\n";
            write(client_socket, response, strlen(response));
        }
    } 
    else {
        char response[] = "HTTP/1.1 404 Not Found\nContent: text/plain\n\nEndpoint not found.\n";
        write(client_socket, response, strlen(response));
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    print_usage();

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
