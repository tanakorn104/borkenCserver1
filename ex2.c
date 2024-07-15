// ได้ครับ ต่อไปนี้คือขั้นตอนการสร้าง Webserver ในภาษาซีบน Windows โดยใช้ socket พร้อมอธิบายฟังก์ชันและพารามิเตอร์ที่ต้องระบุ:

// ### ขั้นตอนที่ 1: การตั้งค่าโปรเจค

// - ติดตั้ง MinGW (หรือ GCC) บน Windows เพื่อคอมไพล์โค้ด C
// - ตรวจสอบให้แน่ใจว่าคุณได้ติดตั้ง Winsock library (ws2_32.lib) ซึ่งเป็นไลบรารีที่จำเป็นสำหรับการทำงานกับ socket บน Windows

// ### ขั้นตอนที่ 2: การเขียนโค้ด Webserver

// #### 1. เริ่มต้น Winsock

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int ex2() {
    WSADATA wsa;

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    // ขั้นตอนอื่นๆ จะอยู่ที่นี่
    WSACleanup(); // ทำความสะอาด Winsock
    return 0;


// ฟังก์ชัน WSAStartup ทำการเริ่มต้น Winsock library:
// - MAKEWORD(2, 2): ระบุเวอร์ชันของ Winsock ที่ต้องการใช้ (2.2)
// - &wsa: โครงสร้าง WSADATA ที่จะรับข้อมูลเกี่ยวกับ Winsock

// #### 2. สร้างและตั้งค่า socket

SOCKET server_socket;
struct sockaddr_in server;

// สร้าง socket
if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    printf("Could not create socket: %d\n", WSAGetLastError());
    return 1;
}

// ตั้งค่า sockaddr_in structure
server.sin_family = AF_INET;
server.sin_addr.s_addr = INADDR_ANY;
server.sin_port = htons(8888);

// ฟังก์ชัน socket สร้าง socket:
// - AF_INET: ใช้ IPv4
// - SOCK_STREAM: ใช้ TCP
// - 0: โปรโตคอลเริ่มต้น

// #### 3. การ bind socket

// Bind socket
if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
    printf("Bind failed with error code: %d\n", WSAGetLastError());
    return 1;
}

// ฟังก์ชัน bind ผูก socket กับพอร์ตและที่อยู่ IP:
// - server_socket: socket ที่สร้างขึ้น
// - (struct sockaddr *)&server: โครงสร้าง sockaddr_in ที่ตั้งค่าแล้ว
// - sizeof(server): ขนาดของโครงสร้าง sockaddr_in

// #### 4. การ listen

// Listen
listen(server_socket, 3);

// ฟังก์ชัน listen ตั้ง socket ให้อยู่ในโหมดรอรับการเชื่อมต่อ:
// - server_socket: socket ที่สร้างขึ้น
// - 3: ขนาดของคิวการเชื่อมต่อที่รอ

// #### 5. การรับการเชื่อมต่อจาก client

SOCKET client_socket;
struct sockaddr_in client;
int client_len = sizeof(struct sockaddr_in);

while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len)) != INVALID_SOCKET) {
    printf("Connection accepted\n");
    handle_request(client_socket);
}


} //อันนร้ปิดให้fn อื่นอยู่ในเมนเฉยๆ
// ฟังก์ชัน accept รอรับการเชื่อมต่อจาก client:
// - server_socket: socket ที่สร้างขึ้น
// - (struct sockaddr *)&client: โครงสร้าง sockaddr_in ที่จะเก็บข้อมูลของ client
// - &client_len: ขนาดของโครงสร้าง sockaddr_in

// #### 6. การจัดการคำขอจาก client

#define BUFFER_SIZE 1024

void handle_request(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int recv_size;

    // รับข้อมูลจาก client
    if ((recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) == SOCKET_ERROR) {
        printf("recv failed\n");
        return;
    }
    buffer[recv_size] = '\0';

    printf("Request received:\n%s\n", buffer);

    // ตรวจสอบประเภทของ request
    if (strncmp(buffer, "GET", 3) == 0 ||
        strncmp(buffer, "POST", 4) == 0 ||
        strncmp(buffer, "PUT", 3) == 0 ||
        strncmp(buffer, "PATCH", 5) == 0) {
        
        const char *response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Connection: close\r\n"
                               "\r\n"
                               "<html><body>Hello, World!</body></html>";
        
        send(client_socket, response, strlen(response), 0);
    }

    // ปิดการเชื่อมต่อกับ client
    closesocket(client_socket);
}

// ฟังก์ชัน recv รับข้อมูลจาก client:
// - client_socket: socket ของ client
// - buffer: บัฟเฟอร์สำหรับเก็บข้อมูลที่รับมา
// - BUFFER_SIZE: ขนาดของบัฟเฟอร์
// - 0: ไม่มี flag พิเศษ

// ฟังก์ชัน send ส่งข้อมูลไปยัง client:
// - client_socket: socket ของ client
// - response: ข้อมูลที่ต้องการส่ง
// - strlen(response): ขนาดของข้อมูลที่ต้องการส่ง
// - 0: ไม่มี flag พิเศษ

// ฟังก์ชัน closesocket ปิดการเชื่อมต่อกับ client:
// - client_socket: socket ของ client

// ### โค้ดทั้งหมด

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define BUFFER_SIZE 1024

void handle_request(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int recv_size;

    // รับข้อมูลจาก client
    if ((recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) == SOCKET_ERROR) {
        printf("recv failed\n");
        return;
    }
    buffer[recv_size] = '\0';

    printf("Request received:\n%s\n", buffer);

    // ตรวจสอบประเภทของ request
    if (strncmp(buffer, "GET", 3) == 0 ||
        strncmp(buffer, "POST", 4) == 0 ||
        strncmp(buffer, "PUT", 3) == 0 ||
        strncmp(buffer, "PATCH", 5) == 0) {
        
        const char *response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Connection: close\r\n"
                               "\r\n"
                               "<html><body>Hello, World!</body></html>";
        
        send(client_socket, response, strlen(response), 0);
    }

    // ปิดการเชื่อมต่อกับ client
    closesocket(client_socket);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_len;

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // สร้าง socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }

    // เตรียม sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d\n", WSAGetLastError());
        return 1;
    }

    // Listen
    listen(server_socket, 3);

    printf("Server listening on port %d\n", PORT);

    // รับการเชื่อมต่อจาก client
    client_len = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len)) != INVALID_SOCKET) {
        printf("Connection accepted\n");
        handle_request(client_socket);
    }

    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed with error code: %d\n", WSAGetLastError());
        return 1;
    }

    // ปิด socket
    closesocket(server_socket);
    WSACleanup();

    return 0;
}

// ### ขั้นตอนที่ 3: คอมไพล์และรันโปรแกรม

// 1. บันทึกโค้ดด้านบนเป็นไฟล์ webserver.c
// 2. เปิด Command Prompt และคอมไพล์โค้ด:
   
//    gcc -o webserver webserver.c -lws2_32
   
// 3. รันไฟล์ที่คอมไพล์แล้ว:
   
//    webserver.exe
   

// ตอนนี้ Webserver จะทำงาน