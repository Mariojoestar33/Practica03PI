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
#define MAX_LINE_LENGTH 1000
#define MAX_MAC_LENGTH 18

// Estructura para almacenar la dirección MAC y su frecuencia
typedef struct {
    char mac[MAX_MAC_LENGTH];
    int frequency;
} MacEntry;

// Función para buscar una dirección MAC en la lista
int findMacIndex(MacEntry *macList, int count, const char *mac) {
    for (int i = 0; i < count; i++) {
        if (strcmp(macList[i].mac, mac) == 0) {
            return i; // La dirección MAC ya está en la lista
        }
    }
    return -1; // La dirección MAC no está en la lista
}

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

    //Inicio de lectura del archivo para sacar las frecuencias

    FILE *arch;
    char line[MAX_LINE_LENGTH];
    char mac_origen[MAX_MAC_LENGTH];
    char mac_destino[MAX_MAC_LENGTH];

    // Arreglo para almacenar las entradas de dirección MAC y su frecuencia
    MacEntry macList[MAX_LINE_LENGTH];
    int macCount = 0;

    // Abre el archivo de texto
    arch = fopen("output.txt", "r");
    if (arch == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Lee el archivo línea por línea
    while (fgets(line, MAX_LINE_LENGTH, arch) != NULL) {
        // Busca las direcciones MAC de origen y destino
        if (sscanf(line, "Dirección MAC de origen: %s", mac_origen) == 1) {
            // Busca si la dirección MAC de origen ya está en la lista
            int index = findMacIndex(macList, macCount, mac_origen);
            if (index == -1) {
                // Si no está, añádela a la lista
                strcpy(macList[macCount].mac, mac_origen);
                macList[macCount].frequency = 1;
                macCount++;
            } else {
                // Si está, incrementa su frecuencia
                macList[index].frequency++;
            }
        } else if (sscanf(line, "Dirección MAC de destino: %s", mac_destino) == 1) {
            // Busca si la dirección MAC de destino ya está en la lista
            int index = findMacIndex(macList, macCount, mac_destino);
            if (index == -1) {
                // Si no está, añádela a la lista
                strcpy(macList[macCount].mac, mac_destino);
                macList[macCount].frequency = 1;
                macCount++;
            } else {
                // Si está, incrementa su frecuencia
                macList[index].frequency++;
            }
        }
    }

    // Cierra el archivo
    fclose(arch);

    // Imprime el recuento de cada dirección MAC
    printf("Frecuencia de cada dirección MAC:\n");
    printf("---------------------------------\n");
    for (int i = 0; i < macCount; i++) {
        printf("%s: %d\n", macList[i].mac, macList[i].frequency);
    }

    // Guardar la información en un archivo de texto
        FILE *archi = fopen("output.txt", "a");
        fprintf(archi, "Frecuencia de cada dirección MAC:\n");
    fprintf(archi, "---------------------------------\n");
    for (int i = 0; i < macCount; i++) {
        fprintf(archi, "%s: %d\n", macList[i].mac, macList[i].frequency);
    }
        fclose(archi);

    return 0;
}