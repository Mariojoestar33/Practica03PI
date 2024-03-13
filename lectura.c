#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1000

int main() {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char trama[MAX_LINE_LENGTH];
    int trama_counter = 0;

    // Abre el archivo de texto
    file = fopen("output.txt", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Lee el archivo línea por línea
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        // Busca la palabra clave que identifica la trama
        if (sscanf(line, "%s", trama) == 1) {
            // Incrementa el contador de la trama correspondiente
            trama_counter++;
        }
    }

    // Cierra el archivo
    fclose(file);

    // Imprime el recuento de tramas
    printf("Frecuencia de tramas:\n");
    printf("---------------------\n");
    printf("Trama: %d\n", trama_counter);

    return 0;
}