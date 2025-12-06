#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include<net/ethernet.h>

// Frame Ethernet + IPv4 + UDP
//  IP src: 192.168.0.50
//  IP dst: 192.168.0.18
//  UDP src port: 4000
//  UDP dst port: 5030
//  payload: "2+2"

unsigned char bytes[] = {
  // ETH (loopback "falso" en Ethernet: mismas MACs)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // MAC destino
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // MAC origen
  0x08, 0x00,                          // EtherType = IPv4

  // IP
  0x45, 0x00,                          // Version/IHL, TOS
  0x00, 0x1f,                          // Total Length = 31 (20 IP + 8 UDP + 3 datos)
  0x00, 0x01,                          // Identification
  0x40, 0x00,                          // Flags/Fragment
  0x40,                                // TTL = 64
  0x11,                                // Protocolo = UDP (17)
  0x3c, 0xcb,                          // Checksum IP (recalculado para 127.0.0.1 -> 127.0.0.1)

  0x7f, 0x00, 0x00, 0x01,              // IP origen = 127.0.0.1
  0x7f, 0x00, 0x00, 0x01,              // IP destino = 127.0.0.1

  // UDP
  0x0f, 0xa0,                          // Puerto origen = 4000
  0x13, 0xa6,                          // Puerto destino = 5030
  0x00, 0x0b,                          // Longitud UDP = 11 (8 + 3)
  0x7a, 0x64,                          // Checksum UDP (recalculado con pseudo-header 127.0.0.1)

  // Payload "2+2"
  0x32, 0x2b, 0x32                     // '2' '+' '2'
};
int main (int argc, char* argv[]) 
{
	int sockfd = socket(AF_PACKET, SOCK_RAW, 0);
	struct sockaddr_ll addr; 
	addr.sll_family = AF_PACKET; 
	addr.sll_ifindex = if_nametoindex("lo"); 
	bind(sockfd, (FAR struct sockaddr *)&addr, sizeof(addr)); /* Bind to device */
	for (int i = 0 ; i < sizeof(bytes) ; ++i){
		printf("%02x", bytes[i]);
	}	
		printf("\n"); 
	send(sockfd, bytes, sizeof(bytes), 0);
	close(sockfd); 
	return 0; 
}



