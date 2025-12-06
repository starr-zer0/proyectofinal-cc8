#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"  
#define SERVER_PORT 5030
#define BUFFER_SIZE 1024

static void log_message(const char* direction, const char* host, const char* type, const char* protocol, const char* desc) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("%s %s %s [%04d/%02d/%02d %02d:%02d:%02d] %s: %s\n",direction, host, type,t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,t->tm_hour, t->tm_min, t->tm_sec,protocol, desc);
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int udp_client_main(int argc, char *argv[])
#endif
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    int bytes_received;
    char local_ip[INET_ADDRSTRLEN] = "127.0.0.1";
    
    printf("Conectado al servidor UDP: %s:%d\n", SERVER_IP, SERVER_PORT);

    // Crear socket UDP 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("ERROR: No se pudo crear el socket UDP\n");
        return 1;
    }
    
    // Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        printf("ERROR: IP del servidor inválida: %s\n", SERVER_IP);
        close(sockfd);
        return 1;
    }
    
    while (1) {
        printf("Ingrese la operación a realizar: ");
        fflush(stdout);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = 0;
        
        // Enviar datagrama
        if (sendto(sockfd, buffer, strlen(buffer), 0,(struct sockaddr*)&server_addr, addr_len) < 0) {
            printf("ERROR: No se pudo enviar el mensaje\n");
            break;
        }
        log_message("<", local_ip, "client", "UDP", buffer);

        // Recibir respuesta 
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,(struct sockaddr*)&server_addr, &addr_len);
        
        if (bytes_received < 0) {
            printf("ERROR: No se pudo recibir respuesta del servidor\n");
            break;
        }
        buffer[bytes_received] = '\0';

        log_message(">", SERVER_IP, "server", "UDP", buffer);
        if (strcmp(buffer, "EXIT") == 0 || strcmp(buffer, "exit") == 0) {
            printf("Saliendo...\n");
            break;
        }
    }
    
    close(sockfd);
    return 0;
}