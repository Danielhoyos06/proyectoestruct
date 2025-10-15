#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <vector>
#include <algorithm>
#include "Fasta.h"

// ----------------------------------------------------
// Función auxiliar: imprime el resultado del comando cargar
// ----------------------------------------------------
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

// ----------------------------------------------------
// Función principal
// ----------------------------------------------------
int main() {
    Fasta db;
    std::string linea;

    while (true) {
        std::cout << "\nComando: ";
        if (!std::getline(std::cin, linea)) break;

        std::istringstream iss(linea);
        std::string cmd;
        iss >> cmd;

        // -----------------------------------------------
        // Comando: cargar nombre_archivo
        // -----------------------------------------------
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
        }

        // -----------------------------------------------
        // Comando: listar_secuencias
        // Regla simple: b = número de caracteres != '-' ; si hay '-' → "al menos b".
        // -----------------------------------------------
        else if (cmd == "listar_secuencias") {
            const auto& seqs = db.secuencias();

            if (seqs.empty()) {
                std::cout << "(no hay secuencias cargadas) No hay secuencias cargadas en memoria.\n";
                continue;
            }

            std::cout << "(resultado exitoso) Hay " << seqs.size()
                      << " secuencias cargadas en memoria:\n";

            for (const auto& s : seqs) {
                const std::string& desc = s.getDescription();
                const std::string& data = s.getData();

                bool tieneGuion = (data.find('-') != std::string::npos);
                size_t b = 0;
                for (unsigned char ch : data) if (ch != '-') ++b;

                if (tieneGuion)
                    std::cout << "Secuencia " << desc << " contiene al menos "
                              << b << " bases.\n";
                else
                    std::cout << "Secuencia " << desc << " contiene "
                              << b << " bases.\n";
            }
        }

        // -----------------------------------------------
        // Comando: es_subsecuencia <subsecuencia>
      
        // -----------------------------------------------
        else if (cmd == "es_subsecuencia") {
            std::string subseq;
            iss >> std::ws;
            std::getline(iss, subseq);

            if (subseq.empty()) {
                std::cout << "Uso: es_subsecuencia <subsecuencia>\n";
                continue;
            }

            const auto& seqs = db.secuencias();
            if (seqs.empty()) {
                std::cout << "(no hay secuencias cargadas) No hay secuencias cargadas en memoria.\n";
                continue;
            }

            size_t total = db.contarSubsecuencia(subseq);

            if (total == 0) {
                std::cout << "(la subsecuencia no existe) La subsecuencia dada no existe dentro de las secuencias cargadas en memoria.\n";
            } else {
                std::cout << "(varias subsecuencias) La subsecuencia dada se repite "
                          << total << " veces dentro de las secuencias cargadas en memoria.\n";
            }
        }

        else if (cmd == "histograma") {
            std::string nombre;
            iss >> std::ws;
            std::getline(iss, nombre);

            if (nombre.empty()) {
                std::cout << "Uso: histograma <descripcion_secuencia>\n";
                continue;
            }

            const auto& seqs = db.secuencias();
            if (seqs.empty()) {
                std::cout << "(la secuencia no existe) Secuencia inválida.\n";
                continue;
            }

            auto hist = db.obtenerHistograma(nombre);
            if (hist.empty()) {
                std::cout << "(la secuencia no existe) Secuencia inválida.\n";
                continue;
            }

            std::cout << "(la secuencia existe)\n";

            // Orden de impresión: A, C, G, T, U, etc.
            const std::vector<char> preferidos = {
                'A','C','G','T','U','R','Y','M','S','W','B','D','H','V','N','X','-'
            };

            for (char k : preferidos) {
                auto it = hist.find(k);
                if (it != hist.end()) {
                    std::cout << k << " : " << it->second << "\n";
                    hist.erase(it);
                }
            }

            // Imprimir cualquier otro símbolo que haya quedado
            if (!hist.empty()) {
                std::vector<std::pair<char, size_t>> resto(hist.begin(), hist.end());
                std::sort(resto.begin(), resto.end(),
                          [](auto& a, auto& b){ return a.first < b.first; });
                for (const auto& [base, freq] : resto)
                    std::cout << base << " : " << freq << "\n";
            }
        }

        // -----------------------------------------------
        // Comando: salir
        // -----------------------------------------------
        else if (cmd == "salir" || cmd == "exit" || cmd == "quit") {
            break;
        }

        // -----------------------------------------------
        // Comando no reconocido
        // -----------------------------------------------
        else if (!cmd.empty()) {
            std::cout << "Comando no reconocido.\n";
        }
    }

    return 0;
}