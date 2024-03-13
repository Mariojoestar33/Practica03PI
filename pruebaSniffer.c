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
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
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

        // Mostrar las direcciones MAC de origen y destino
        printf("Dirección MAC origen: %02x:%02x:%02x:%02x:%02x:%02x\n", 
               eth_header->h_source[0], eth_header->h_source[1], eth_header->h_source[2], 
               eth_header->h_source[3], eth_header->h_source[4], eth_header->h_source[5]);

        printf("Dirección MAC  destino: %02x:%02x:%02x:%02x:%02x:%02x\n", 
               eth_header->h_dest[0], eth_header->h_dest[1], eth_header->h_dest[2], 
               eth_header->h_dest[3], eth_header->h_dest[4], eth_header->h_dest[5]);

        // Analizar el encabezado IP
        struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        unsigned int ip_header_length = ip_header->ihl * 4; // Tamaño del encabezado IP en bytes

        // Mostrar información básica del paquete
        printf("Versión IP: %d\n", ip_header->version);

        // Calcular la longitud de la trama y del campo de datos
        unsigned int frame_length = packet_size;
        unsigned int data_length = frame_length - sizeof(struct ethhdr) - ip_header_length;

        // Mostrar la longitud de la trama y del campo de datos
        printf("Longitud de la trama: %u bytes\n", frame_length);
        printf("Longitud del campo de datos: %u bytes\n", data_length);

        // Guardar la información en un archivo de texto
        FILE *file = fopen("output.txt", "a");
        fprintf(file, "Este es el paquete número %d:\n", packet_count + 1);
        fprintf(file, "Dirección MAC de origen: %02x:%02x:%02x:%02x:%02x:%02x\n", 
               eth_header->h_source[0], eth_header->h_source[1], eth_header->h_source[2], 
               eth_header->h_source[3], eth_header->h_source[4], eth_header->h_source[5]);
        fprintf(file, "Dirección MAC de destino: %02x:%02x:%02x:%02x:%02x:%02x\n", 
               eth_header->h_dest[0], eth_header->h_dest[1], eth_header->h_dest[2], 
               eth_header->h_dest[3], eth_header->h_dest[4], eth_header->h_dest[5]);
        fprintf(file, "Versión IP: %d\n", ip_header->version);
        fprintf(file, "Longitud de la trama: %u bytes\n", frame_length);
        fprintf(file, "Longitud del campo de datos: %u bytes\n\n", data_length);
        fclose(file);

        packet_count++;
    }

    // Cerrar el socket
    close(raw_socket);

    return 0;
}