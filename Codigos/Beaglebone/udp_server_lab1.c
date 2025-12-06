/****************************************************************************
 * apps/examples/udp_server/udp_server_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5030
#define BUFFER_SIZE 1024

static void log_message(const char* direction, const char* host, const char* type,const char* protocol, const char* desc) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("%s %s %s [%04d/%02d/%02d %02d:%02d:%02d] %s: %s\n",direction, host, type,t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,t->tm_hour, t->tm_min, t->tm_sec,protocol, desc);
}

char* evaluate_expression(const char* expr) {
    static char result[64];
    int a, b;
    char op;
    
    if (sscanf(expr, "%d %c %d", &a, &op, &b) == 3) {
        // Éxito con espacios
    } else if (sscanf(expr, "%d%c%d", &a, &op, &b) == 3) {
        // Éxito sin espacios
    } else {
        strcpy(result, "Error: Formato inválido");
        return result;
    }
    
    switch (op) {
        case '+': snprintf(result, sizeof(result), "%d", a + b); break;
        case '-': snprintf(result, sizeof(result), "%d", a - b); break;
        case '*': snprintf(result, sizeof(result), "%d", a * b); break;
        case '/': 
            if (b == 0) strcpy(result, "Error: División por cero");
            else snprintf(result, sizeof(result), "%d", a / b);
            break;
        case '%': 
            if (b == 0) strcpy(result, "Error: Módulo por cero");
            else snprintf(result, sizeof(result), "%d", a % b);
            break;
        default: strcpy(result, "Operación inválida");
    }
    
    return result;
}

int udp_server_main(int argc, FAR char *argv[])
{
    printf("=== Servidor UDP (Loopback) ===\n");
    printf("Iniciando en 127.0.0.1:%d\n", PORT);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("ERROR: Socket failed\n");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
    
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("ERROR: Bind failed\n");
        close(sockfd);
        return 1;
    }
    
    printf("Servidor listo. Esperando conexiones...\n\n");
    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    while (1) {
        int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0,(struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            
            log_message(">", client_ip, "client", "UDP", buffer);
            
            char *response;
            if (strcmp(buffer, "EXIT") == 0) {
                response = "EXIT";
            } else {
                response = evaluate_expression(buffer);
            }
            
            sendto(sockfd, response, strlen(response), 0,
                  (struct sockaddr*)&client_addr, client_len);
                  
            log_message("<", "127.0.0.1", "server", "UDP", response);
            
            printf("Procesado: %s -> %s\n\n", buffer, response);
        }
    }
    
    close(sockfd);
    return 0;
}