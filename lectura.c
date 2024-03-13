#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char mac_origen[MAX_MAC_LENGTH];
    char mac_destino[MAX_MAC_LENGTH];

    // Arreglo para almacenar las entradas de dirección MAC y su frecuencia
    MacEntry macList[MAX_LINE_LENGTH];
    int macCount = 0;

    // Abre el archivo de texto
    file = fopen("output.txt", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Lee el archivo línea por línea
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
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
    fclose(file);

    // Imprime el recuento de cada dirección MAC
    printf("Frecuencia de cada dirección MAC:\n");
    printf("---------------------------------\n");
    for (int i = 0; i < macCount; i++) {
        printf("%s: %d\n", macList[i].mac, macList[i].frequency);
    }

    return 0;
}
