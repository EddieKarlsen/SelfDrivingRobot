// Lägg till dessa funktioner i din main.c

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>


// Hjälpfunktion för att kolla om Python-skript finns
int check_python_script() {
    struct stat buffer;
    if (stat("analysis/heatmap_generator.py", &buffer) != 0) {
        printf("Python-skript 'analysis/heatmap_generator.py' hittades inte!\n");
        printf("Kontrollera att analysis-mappen och skriptet finns.\n");
        return 0;
    }
    return 1;
}

// Hjälpfunktion för att kolla om Python finns
int check_python_installation() {
    int result = system("python3 --version > /dev/null 2>&1");
    if (result != 0) {
        result = system("python --version > /dev/null 2>&1");
        if (result != 0) {
            printf(" Python hittades inte! Installera Python3 först.\n");
            return 0;
        }
    }
    return 1;
}
