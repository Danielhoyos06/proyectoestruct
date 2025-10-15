#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include "Fasta.h"

static void imprimirResultadoCargar(const std::string& nombre, size_t count) {
    if (count == std::numeric_limits<size_t>::max()) {
        std::cout << "(archivo erróneo) " << nombre
                  << " no se encuentra o no puede leerse.\n";
    } else if (count == 0) {
        std::cout << "(archivo vacío) " << nombre
                  << " no contiene ninguna secuencia.\n";
    } else if (count == 1) {
        std::cout << "(una sola secuencia) 1 secuencia cargada correctamente desde "
                  << nombre << " .\n";
    } else {
        std::cout << "(varias secuencias) " << count
                  << " secuencias cargadas correctamente desde "
                  << nombre << " .\n";
    }
}

int main() {
    Fasta db;
    std::string linea;

    while (true) {
        std::cout << "\nComando: ";
        if (!std::getline(std::cin, linea)) break;

        std::istringstream iss(linea);
        std::string cmd;
        iss >> cmd;

        if (cmd == "cargar") {
            std::string nombre;
            iss >> std::ws;
            std::getline(iss, nombre);

            if (nombre.empty()) {
                std::cout << "Uso: cargar <nombre_archivo>\n";
                continue;
            }

            size_t count = db.cargar(nombre);
            imprimirResultadoCargar(nombre, count);
        } else if (cmd == "salir" || cmd == "exit" || cmd == "quit") {
            break;
        } else if (!cmd.empty()) {
            std::cout << "Comando no reconocido.\n";
        }
    }

    return 0;
}
