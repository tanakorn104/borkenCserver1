#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#pragma comment(lib, "ws2_32")
#define sizeofbuffer 4096   //and message
char buffer[sizeofbuffer];
char response[sizeofbuffer];

typedef struct{
    SOCKET thclient_socket;
    char thclient_message[sizeofbuffer];
}ThreadParams;


// void openfile(char pathfilename,SOCKET client_socket){
    //หากเปิดได้ คืนค่า fopen ออกไปใช้ต่อ
    // FILE *internalfile;
    // internalfile = fopen(pathfilename,"rb");
    // if(internalfile==NULL){
        // char *response_header = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
        // char *response_body = "<html>\n"
                    // "<head><title>Internal Server Error</title></head>\n"
                    // "<body>\n"
                    // "<h1>Somthing wrong with server</h1>\n"
                    // "</body>\n"
                    // "</html>\n";
// 
        // send(client_socket, response_header, strlen(response_header), 0);
        // send(client_socket, response_body, strlen(response_body), 0);
        // return -1;
    // }
    // return internalfile;
// 
// }

void sendclientfile(SOCKET client_socket,char *contentType,FILE *internalfile){
            

            // สร้าง HTTP response แบบ chunked
            sprintf(response, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: %s\r\n"
                      "Transfer-Encoding: chunked\r\n\r\n",contentType);
            
            send(client_socket, response, strlen(response), 0);
            //snprintf(response,"HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",sizeofbuffer); //เริ่มจากการส่ง http200 เพื่อให้เข้าถึงhttp request ของ client ได้
            size_t sizeofchunk;
            char chunkheader[32];
            int z =0;
            printf("start chunked \n");
            while((sizeofchunk=fread(buffer,1,sizeofbuffer,internalfile))>0){
                //Ternary Operator  อันนี้ตรวจสอบว่า htmlsize มากกว่าbuffer(ค่าที่จะส่งในแต่ละครั้ง)ไหม 
                //หากใช่chucksize = buffer หากไม่ chucksize = htmlsize ที่เหลืออยู่
                int chunkSize = (sizeofchunk<sizeofbuffer)?sizeofchunk:sizeofbuffer;

                //สร้างchunk header เพื่อบอกขนาดของข้อมูลที่ส่งไป
                sprintf(chunkheader,"%x\r\n",chunkSize);
                send(client_socket,chunkheader,strlen(chunkheader),0);
                //ที่ต้องส่งข้อมูลในขนาดเท่ากับ chunk เพราะว่าในกรณีที่ข้อมูลที่อ่านได้น้อยกว่าขนาดของbuffer จะได้ส่งไปเฉพาะข้อมูลที่อ่านได้
                send(client_socket,buffer,chunkSize,0);
                send(client_socket,"\r\n",2,0);
                printf("Send Chunk :%d\n",z);
                puts(buffer);
                // memset(buffer,0,sizeof(buffer));
                
                
                // sizeofchunk-=sizeofbuffer;

            }
            send(client_socket,"0\r\n\r\n",5,0);
            printf("end Chunk\n");
            fclose(internalfile);
}


DWORD WINAPI handle_request(LPVOID inparams){//clientmsg clientsock
    // char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"; //เริ่มจากการส่ง http200 เพื่อให้เข้าถึงhttp request ของ client ได้
    // send(client_socket, response, strlen(response), 0);
    ThreadParams *ptr =  (ThreadParams*)inparams;
    // SOCKET client_socket = ptr->client_socket;
    // char client_message = ptr->client_message;

    // int recv_clientsize;
    // recv_clientsize = recv(client_socket,client_message,sizeofbuffer,0);
    // client_message[recv_clientsize]='\0';

    // char *message_servertoclient;
    // puts("\nRecive request : %s \n",client_message);
    // puts(client_message);
    // message_servertoclient = "Hello client";
    // if((send(client_socket,message_servertoclient,strlen(message_servertoclient),0))==INVALID_SOCKET){
    //     printf("\nSomething wrong with sending message to client");
    // }
    char method[16],querystrn[1024],httpver[16] ;
    FILE *file;
    puts(ptr->thclient_message);
    sscanf(ptr->thclient_message, "%s %s %s", method,querystrn,httpver);
    printf("%s\n%s\nlenqustr:%d\n\n\n\n",method,querystrn,strlen(querystrn));
    if(strncmp("GET",method,strlen("GET"))==0 ){
         //ถ้าquery มาแบบ registersmlfmf แก้ปัญหาด้วยการเพิ่มฟังชั่นเปิดไฟล์จาก querystr หาากเปิดไม่ได้ก็404
        char pathfile[30];
        char content_type[30];
        //สร้าง path file
        if(strstr(querystrn,".css")!=NULL){
            sprintf(pathfile,"src/component%s",querystrn);
            strncat(content_type,"text/css",10);
        }else if(strstr(querystrn,".js")!=NULL){
            sprintf(pathfile,"src/component%s",querystrn);
            strncat(content_type,"text/javascript",17);
        }else{//ปัญหาคือ ผู้ใช้ระบุ / แปลว่าตัวการ index แต่ถ้าเปรียบเทียบแค่ตัวแรกทุกคำขอเช่น /register ก็จะได้ index
            // printf("pathfilelen :%d",strlen(pathfile));
            if(strlen(querystrn)==1){//รวม\0
                strncat(pathfile,"src/index.html",20);
                strncat(content_type,"text/html",11);
            }else{
                sprintf(pathfile,"src%s.html",querystrn);
                strncat(content_type,"text/html",11);

            }
        }

        FILE *internalfile;
        printf("pathfile :%s \ncontent_type :%s\n",pathfile,content_type);
        internalfile = fopen(pathfile,"rb");
        if(internalfile==NULL){
            char *response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
            char *response_body = "<html>\n"
                            "<head><title>404 Not Found</title></head>\n"
                            "<body>\n"
                            "<h1>404 Not Found</h1>\n"
                            "<p>The requested URL was not found on this server.</p>\n"
                            "</body>\n"
                            "</html>\n";

            send(ptr->thclient_socket, response_header, strlen(response_header), 0);
            send(ptr->thclient_socket, response_body, strlen(response_body), 0);
            
            // char *response_header = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
            // char *response_body = "<html>\n"
            //             "<head><title>Internal Server Error</title></head>\n"
            //             "<body>\n"
            //             "<h1>Somthing wrong with server</h1>\n"
            //             "</body>\n"
            //             "</html>\n";

            // send(client_socket, response_header, strlen(response_header), 0);
            // send(client_socket, response_body, strlen(response_body), 0);

        }else{
            sendclientfile(ptr->thclient_socket,content_type,internalfile);
            // if(strncmp(querystrn,"/register",strlen("/register"))==0){ //ในกรณีที่client ใช้ /registersdfksdf การเปลี่ยนเทียบจะได้ไม่เท่ากัน
            //     sendclientfile(client_socket,"text/html",internalfile);

            // }else if(strncmp(querystrn,"/style.css",strlen(querystrn))==0){//ในกรณที่ user ส่ง/อะไรไม่รู้ กับ /registersosgmsk ทำให้คืนเป็นหน้า404
            //     sendclientfile(client_socket,"text/css",internalfile);

            // }else if(strncmp(querystrn,"/",strlen(querystrn))==0){//defultpage index
            //     sendclientfile(client_socket,"text/html",internalfile);

            // }else{
                
            // }
    }



        }

}

int main(){

    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2,0),&wsadata) !=0){
        printf("WSA Initialization failed.");
        return 0;
    }
    
    struct sockaddr_in serveraddr,clientaddr; //เป็นโครงสร้างที่ใช้เก็บข้อมูลที่อยู่ของserver และจะเก็บของ cleint เมื่อมีการขอเชื่อมต่อเข้ามา
    serveraddr.sin_family =AF_INET;
    serveraddr.sin_addr.s_addr =inet_addr("127.0.0.1");
    serveraddr.sin_port=htons(5543);

    //create socket
    
    SOCKET server_socket,client_socket;
    
    if((server_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET){
        printf("Create socket failed");
        return 0;
    }
    
    //bind
    if((bind(server_socket,(struct sockaddr *)&serveraddr,sizeof(serveraddr)))==SOCKET_ERROR){
        printf("binding server failed");
        return 0;
    }


    listen(server_socket,15);

    int i=0;
    while(1){
        if(i==2){
            return 0;
        }
        clock_t start,end;
        double cpu_time_used;
        start = clock();
        printf("------------------------------------Waiting for client. . %d------------------------------------\n",i);
        int clientlen;
        clientlen =sizeof(struct sockaddr_in);
        void*ptr = &client_socket;
        client_socket = accept(server_socket,(struct sockaddr *)&clientaddr,&clientlen);// socket ที่listenอยู่ , ตัวแปรที่จะเก็บclientaddr , ขนาด sockaddr in
        //accept จะคืนค่าที่อยู่ของsocket clientใหม่ที่จะใช้คุยกัน
        // handle_request(client_socket);
        //thread
        char client_message[sizeofbuffer];
        int recv_clientsize;
        recv_clientsize = recv(client_socket,client_message,sizeofbuffer,0);
        client_message[recv_clientsize]='\0';
        puts(client_message);
        //ทำการเช็คว่า คำขอเป็นของclient จริงไหม จาก GET POST
        ThreadParams params={client_socket,*client_message};
        DWORD dwThreadID;
        HANDLE hThread =CreateThread(
            NULL, //security
            0, //default stacksize
            handle_request,
            &params,
            0,
            &dwThreadID
        );
        printf("ThreadId :%lu \n",dwThreadID);
        WaitForSingleObject(hThread,INFINITE);
        CloseHandle(hThread);

        closesocket(client_socket);
        closesocket(server_socket);
        WSACleanup();

        i++;
        end = clock();
        cpu_time_used = ((double)(end-start))/CLOCKS_PER_SEC;
        printf("run time : %f \n",cpu_time_used);

    }






    return 0;
}