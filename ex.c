#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h> // For _beginthread and _endthread

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"
#define BUFFER_SIZE 1024

void handle_client(void *client_socket);

int main() {
    WSADATA wsaData;
    int iResult;
    struct addrinfo *result = NULL, hints;
    SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Setup the hints address info structure
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a socket to listen for connections
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Socket failed: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Bind the socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("Bind failed: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    // Listen for incoming connections
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("Listen failed: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("Listening on port %s...\n", DEFAULT_PORT);

    // Accept incoming connections and handle them in a new thread
    while (1) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("Accept failed: %ld\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // Create a new thread to handle the client
        _beginthread(handle_client, 0, (void *)ClientSocket);
    }

    // Cleanup
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}

void handle_client(void *client_socket) {
    SOCKET ClientSocket = (SOCKET)client_socket;
    char recvbuf[BUFFER_SIZE];
    int iResult;
    char sendbuf[BUFFER_SIZE];

    // Receive data from the client
    iResult = recv(ClientSocket, recvbuf, BUFFER_SIZE, 0);
    if (iResult > 0) {
        recvbuf[iResult] = '\0';
        printf("Received: %s\n", recvbuf);

        // Simple HTTP response handling
        if (strstr(recvbuf, "GET / ") != NULL) {
            snprintf(sendbuf, BUFFER_SIZE, 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "<html><body><h1>Hello, World!</h1></body></html>", 44);
        } else if (strstr(recvbuf, "GET /style.css ") != NULL) {
            snprintf(sendbuf, BUFFER_SIZE, 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/css\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "body { background-color: lightblue; }", 42);
        } else if (strstr(recvbuf, "GET /script.js ") != NULL) {
            snprintf(sendbuf, BUFFER_SIZE, 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/javascript\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "alert('Hello, World!');", 23);
        } else {
            snprintf(sendbuf, BUFFER_SIZE, 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "404 Not Found", 13);
        }

        // Send the response to the client
        send(ClientSocket, sendbuf, strlen(sendbuf), 0);
    }

    // Cleanup
    closesocket(ClientSocket);
    _endthread();
}