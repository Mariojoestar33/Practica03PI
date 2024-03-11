#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <netinet/ether.h> // Para trabajar con direcciones MAC

#define BUFFER_SIZE 65536

int main() {
    char interface_name[IFNAMSIZ];
    int num_packets;

    // Solicitar al usuario el nombre de la interfaz de red
    printf("Ingrese el nombre de la interfaz de red: ");
    scanf("%s", interface_name);

    // Solicitar al usuario el número de paquetes a analizar
    printf("Ingrese el número de paquetes a analizar (-1 para analizar todos los paquetes): ");
    scanf("%d", &num_packets);

    int raw_socket;
    unsigned char buffer[BUFFER_SIZE];
    struct ifreq ifr;

    // Crear un socket raw
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // Cambiado a ETH_P_ALL para capturar todos los tipos de protocolo Ethernet
    if (raw_socket < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar el modo promiscuo
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
    if (setsockopt(raw_socket, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("Error al configurar el modo promiscuo");
        exit(EXIT_FAILURE);
    }

    // Leer los paquetes capturados
    int packet_count = 0;
    while (packet_count < num_packets || num_packets == -1) {
        ssize_t packet_size = recv(raw_socket, buffer, BUFFER_SIZE, 0);
        if (packet_size < 0) {
            perror("Error al recibir el paquete");
            exit(EXIT_FAILURE);
        }

        // Analizar el encabezado Ethernet
        struct ethhdr *eth_header = (struct ethhdr *)buffer;

        // Verificar si es un paquete IPv4 o IPv6
        if (ntohs(eth_header->h_proto) == ETH_P_IP) {
            printf("Este es un paquete IPv4.\n");
        } else if (ntohs(eth_header->h_proto) == ETH_P_IPV6) {
            printf("Este es un paquete IPv6.\n");
        } else {
            printf("Este paquete no es IPv4 ni IPv6.\n");
            continue; // Saltar al siguiente paquete
        }

        // Mostrar las direcciones MAC de origen y destino
        printf("Dirección MAC de origen: %02x:%02x:%02x:%02x:%02x:%02x\n", 
               eth_header->h_source[0], eth_header->h_source[1], eth_header->h_source[2], 
               eth_header->h_source[3], eth_header->h_source[4], eth_header->h_source[5]);

        printf("Dirección MAC de destino: %02x:%02x:%02x:%02x:%02x:%02x\n", 
               eth_header->h_dest[0], eth_header->h_dest[1], eth_header->h_dest[2], 
               eth_header->h_dest[3], eth_header->h_dest[4], eth_header->h_dest[5]);

        // Analizar el encabezado IP
        struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        unsigned int ip_header_length = ip_header->ihl * 4; // Tamaño del encabezado IP en bytes

        // Mostrar información básica del paquete
        printf("Versión IP: %d\n", ip_header->version);
        printf("Longitud del encabezado IP: %d bytes\n", ip_header_length);
        printf("Protocolo: %d\n", ip_header->protocol);
        // Puedes agregar más información según tus necesidades

        // Guardar la información en un archivo de texto
        FILE *file = fopen("output.txt", "a");
        fprintf(file, "Versión IP: %d\n", ip_header->version);
        fprintf(file, "Longitud del encabezado IP: %d bytes\n", ip_header_length);
        fprintf(file, "Protocolo: %d\n", ip_header->protocol);
        // Agrega más información según tus necesidades
        fclose(file);

        packet_count++;
    }

    // Cerrar el socket
    close(raw_socket);

    return 0;
}

