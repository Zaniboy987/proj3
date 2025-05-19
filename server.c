#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define MAX_TODO 10
#define MAX_LISTS 10
#define BUFFER_SIZE 1024

volatile sig_atomic_t running = 1;

typedef struct {
    char task[256];
    int completed;
} Todo;

// Todo todo_list[MAX_TODO];
// int todo_count = 0;
typedef struct {
    char name[50];
    Todo tasks[MAX_TODO];
    int task_count;
} TodoList;

TodoList todo_lists[MAX_LISTS];
int list_count = 0;

TodoList* get_list_by_name(const char* name) {
    for (int i = 0; i < list_count; i++) {
        if (strcmp(todo_lists[i].name, name) == 0) {
            return &todo_lists[i];
        }
    }
    return NULL;
}

TodoList* create_list(const char* name) {
    if (list_count < MAX_LISTS && get_list_by_name(name) == NULL) {
        strncpy(todo_lists[list_count].name, name, sizeof(todo_lists[list_count].name) - 1);
        todo_lists[list_count].task_count = 0;
        return &todo_lists[list_count++];
    }
    return NULL;
}

void print_usage() {
    printf("Server is running on port %d\n", PORT);
    printf("\n********** TODO API Server **********\n");

    printf("1. Create a New List:\n");
    printf("   curl -X POST http://localhost:%d/lists/<list>\n", PORT);

    printf("\n2. View All Lists:\n");
    printf("   curl http://localhost:%d/lists\n", PORT);

    printf("\n3. Delete a List:\n");
    printf("   curl -X DELETE http://localhost:%d/lists/<list>\n", PORT);
    
    printf("\n4. Add Task to List: \n");
    printf("   curl -X POST -d \"Your task description\" http://localhost:%d/todos/<list>\n", PORT);

    printf("\n5. View All Tasks in a List: \n");
    printf("   curl http://localhost:%d/todos/<list>\n", PORT);

    printf("\n6. Complete a Task (by index) in a List: \n");
    printf("   curl -X PUT http://localhost:%d/todos/<list>/<index>/complete\n", PORT);
    // printf("   Example: curl -X PUT http://localhost:%d/todos/1/complete\n", PORT);

    printf("\n7. Delete a Task (by index) in a List: \n");
    printf("   curl -X DELETE http://localhost:%d/todos/<list>/<index>\n", PORT);
    //printf("   Example: curl -X DELETE http://localhost:%d/todos/1\n", PORT);

    printf("\n8. View Completed Tasks from a List: \n");
    printf("   curl http://localhost:%d/todos/<list>/history\n", PORT);

    printf("\n9. Clear All Completed Tasks in a List:\n");
    printf("   curl -X DELETE http://localhost:%d/todos/<list>/completed\n", PORT);

    printf("\n10. Shutdown Server:\n");
    printf("   curl http://localhost:%d/shutdown\n", PORT);
    
    printf("********************************************\n\n");
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
    buffer[bytes_read] = '\0';

    char* list_name = NULL;
    char* subpath = NULL;
    TodoList* list = NULL;

    //SHUTDOWN server
    char method[8], path[256];
    sscanf(buffer, "%s %s", method, path);

    if (strcmp(method, "GET") == 0 && strcmp(path, "/shutdown") == 0) {
        const char* msg = "HTTP/1.1 200 OK\nContent: text/plain\n\nShutting down server.\n";
        write(client_socket, msg, strlen(msg));
        running = 0;
        close(client_socket);
        return;
    }



    // Handle /lists routes
    // CREATE new list
    if (strncmp(buffer, "POST /lists/", 12) == 0) {
        char* name = buffer + 12;
        char* end = strchr(name, ' ');
        if (end) *end = '\0';
        TodoList* new_list = create_list(name);
        const char* response = new_list ?
            "HTTP/1.1 201 Created\nContent: text/plain\n\nList created.\n" :
            "HTTP/1.1 400 Bad Request\nContent: text/plain\n\nList already exists or limit reached.\n";
        write(client_socket, response, strlen(response));
    }
    // VIEW all lists
    else if (strncmp(buffer, "GET /lists", 10) == 0) {
        char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\nContent: text/plain\n\nYour Lists:\n";
        for (int i = 0; i < list_count; i++) {
            strcat(response, "- ");
            strcat(response, todo_lists[i].name);
            strcat(response, "\n");
        }
        write(client_socket, response, strlen(response));
    }
    // DELETE a list
    else if (strncmp(buffer, "DELETE /lists/", 14) == 0) {
        char* name = buffer + 14;
        char* end = strchr(name, ' ');
        if (end) *end = '\0';
        int found = 0;
        for (int i = 0; i < list_count; i++) {
            if (strcmp(todo_lists[i].name, name) == 0) {
                for (int j = i; j < list_count - 1; j++) {
                    todo_lists[j] = todo_lists[j + 1];
                }
                list_count--;
                found = 1;
                break;
            }
        }
        const char* msg = found ?
            "HTTP/1.1 200 OK\nContent: text/plain\n\nList deleted.\n" :
            "HTTP/1.1 404 Not Found\nContent: text/plain\n\nList not found.\n";
        write(client_socket, msg, strlen(msg));
    }

    // Which list command is for
    else if (strncmp(buffer, "GET /todos/", 11) == 0 || strncmp(buffer, "POST /todos/", 12) == 0 ||
        strncmp(buffer, "PUT /todos/", 11) == 0 || strncmp(buffer, "DELETE /todos/", 14) == 0) {
        char method[8], path[256];
        sscanf(buffer, "%s %s", method, path);
        if (strncmp(path, "/todos/", 7) == 0) {
            list_name = path + 7;
            subpath = strchr(list_name, '/');
            if (subpath) *subpath++ = '\0'; 
        }

        list = get_list_by_name(list_name);
        // printf("Parsed list name: '%s'\n", list_name);
        // if (subpath) printf("Parsed subpath: '%s'\n", subpath);

        if (!list) {
            const char* msg = "HTTP/1.1 404 Not Found\nContent: text/plain\n\nList not found.\n";
            write(client_socket, msg, strlen(msg));
        }
        // VIEW COMPLETED tasks in list
        else if (strstr(buffer, "GET /todos/") && subpath && strcmp(subpath, "history") == 0) {
            char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\nContent: text/plain\n\n";
            for (int i = 0; i < list->task_count; i++) {
                if (list->tasks[i].completed) {
                    char todo_item[300];
                    snprintf(todo_item, sizeof(todo_item), "%d. %s [Completed]\n", i + 1, list->tasks[i].task);
                    strcat(response, todo_item);
                }
            }
            write(client_socket, response, strlen(response));
        } 
        // VIEW ALL tasks in list
        else if (strstr(buffer, "GET /todos/")) {
            char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\nContent: text/plain\n\n";
            for (int i = 0; i < list->task_count; i++) {
                char todo_item[300];
                snprintf(todo_item, sizeof(todo_item), "%d. %s [%s]\n", i + 1, list->tasks[i].task, list->tasks[i].completed ? "Completed" : "Pending");
                strcat(response, todo_item);
            }
            write(client_socket, response, strlen(response));
        } 
        // ADD task to list
        else if (strstr(buffer, "POST /todos/")) {
            char *task_start = strstr(buffer, "\r\n\r\n");
            if (task_start) {
                task_start += 4;
                // printf("Task extracted: '%s'\n", task_start);
            }

            if (task_start && list->task_count < MAX_TODO) {
                strncpy(list->tasks[list->task_count].task, task_start, 255);
                list->tasks[list->task_count].completed = 0;
                list->task_count++;

                char response[] = "HTTP/1.1 201 Created\nContent: text/plain\n\nTask added.\n";
                write(client_socket, response, strlen(response));
            } else {
                char response[] = "HTTP/1.1 400 Bad Request\nContent: text/plain\n\nInvalid request or list full.\n";
                write(client_socket, response, strlen(response));
            }
        } 
        // MARK task as COMPLETE
        else if (strstr(buffer, "PUT /todos/") && strstr(buffer, "/complete")) {
            int index = atoi(subpath) - 1;
            if (index >= 0 && index < list->task_count) {
                list->tasks[index].completed = 1;
                char response[] = "HTTP/1.1 200 OK\nContent: text/plain\n\nTask marked as completed.\n";
                write(client_socket, response, strlen(response));
            } else {
                char response[] = "HTTP/1.1 404 Not Found\nContent: text/plain\n\nTask not found.\n";
                write(client_socket, response, strlen(response));
            }
        } 
        // DELETE all COMPLETED tasks
        else if (strncmp(buffer, "DELETE /todos/", 14) == 0 && strstr(buffer, "/completed")) {
            int write_idx = 0;
            for (int read_idx = 0; read_idx < list->task_count; read_idx++) {
                if (!list->tasks[read_idx].completed) {
                    list->tasks[write_idx++] = list->tasks[read_idx];
                }
            }
            list->task_count = write_idx;
            char response[] = "HTTP/1.1 200 OK\nContent: text/plain\n\nCompleted tasks cleared.\n";
            write(client_socket, response, strlen(response));
        }
        // DELETE a task
        else if (strncmp(buffer, "DELETE /todos/", 14) == 0) {
            int index = atoi(subpath) - 1;
            if (index >= 0 && index < list->task_count) {
                for (int i = index; i < list->task_count - 1; i++) {
                    list->tasks[i] = list->tasks[i + 1];
                }
                list->task_count--;
                char response[] = "HTTP/1.1 200 OK\nContent: text/plain\n\nTask deleted.\n";
                write(client_socket, response, strlen(response));
            } else {
                char response[] = "HTTP/1.1 404 Not Found\nContent: text/plain\n\nTask not found.\n";
                write(client_socket, response, strlen(response));
            }
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

    while (running) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            if (running) perror("Accept failed");
            continue;
        }
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
