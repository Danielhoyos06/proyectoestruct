#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <vector>
#include <algorithm>
#include "Fasta.h"


static void mostrarMenu() {
    std::cout
        << "================= MENU =================\n"
        << "Comandos disponibles:\n"
        << "  cargar <nombre_archivo>        : Carga un archivo FASTA en memoria\n"
        << "  listar_secuencias              : Muestra la cantidad y longitud de cada secuencia\n"
        << "  es_subsecuencia <subsecuencia> : Cuenta repeticiones de una subsecuencia (solapadas)\n"
        << "  histograma <descripcion>       : Imprime el histograma de la secuencia indicada\n"
        << "  enmascarar <subsecuencia>      : Reemplaza cada ocurrencia por 'X' en todas las secuencias\n"
        << "  guardar <nombre_archivo>       : Guarda las secuencias actuales en un archivo FASTA\n"
        << "  codificar <nombre_archivo.fabin> : Codificar secuencias en binario Huffman\n"
        << "decodificar <archivo.fabin>\n"
        << "  salir                          : Termina el programa\n"
        << "========================================\n";
}


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


int main() {
    Fasta db;
    std::string linea;

    mostrarMenu();

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
        }// se uso una herramineta de IA (chatgpt) para la elaboracion del esqueleto de esta funcion(funcion num 3 de las funciopnes del TAD Fasta) con el prompt de "teniendo en cuenta esta funcion(funcion), genera el esqueleto para su implementacion en el siguiente main (todo el main) "

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

            const std::vector<char> preferidos = {
                'A','C','G','T','U','R','Y','M','S','W','B','D','H','V','N','X','-'
            }; //se usa para mostrar el orden de impresion del histograma 

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

          else if (cmd == "enmascarar") {
            std::string subseq;
            iss >> std::ws;
            std::getline(iss, subseq);

            if (subseq.empty()) {
                std::cout << "Uso: enmascarar <subsecuencia>\n";
                continue;
            }

            const auto& seqs = db.secuencias();
            if (seqs.empty()) {
                std::cout << "(no hay secuencias cargadas) No hay secuencias cargadas en memoria.\n";
                continue;
            }

            size_t s = db.enmascararSubsecuencia(subseq);

            if (s == 0) {
                std::cout << "(no se enmascararon subsecuencias) La subsecuencia dada no existe dentro de las secuencias cargadas en memoria, por tanto no se enmascara nada.\n";
            } else {
                std::cout << "(varias subsecuencias esmascaradas) " // (respeto la ortografía exacta que pediste)
                          << s << " subsecuencias han sido enmascaradas dentro de las secuencias cargadas en memoria.\n";
            }
        }
        else if (cmd == "guardar") {
            std::string nombre;
            iss >> std::ws;
            std::getline(iss, nombre);
            if (nombre.empty()) {
                std::cout << "Uso: guardar <nombre_archivo>\n";
                continue;
            }

            const auto& seqs = db.secuencias();
            if (seqs.empty()) {
                std::cout << "(no hay secuencias cargadas) No hay secuencias cargadas en memoria.\n";
                continue;
            }

            bool ok = db.guardar(nombre);
            if (ok)
                std::cout << "(escritura exitosa) Las secuencias han sido guardadas en "
                          << nombre << " .\n";
            else
                std::cout << "(problemas en archivo) Error guardando en "
                          << nombre << " .\n";
        }

        else if (cmd == "salir" || cmd == "exit" || cmd == "quit") {
            break;
        }

       
        else if (cmd == "codificar") {
            std::string nombre;
            iss >> std::ws;
            std::getline(iss, nombre);
            if (nombre.empty()) {
                std::cout << "Uso: codificar <nombre_archivo.fabin>\n";
                continue;
            }
            const auto& seqs = db.secuencias();
            if (seqs.empty()) {
                std::cout << "(no hay secuencias cargadas) No hay secuencias cargadas en memoria.\n";
                continue;
            }
            bool ok = db.codificarHuffman(nombre);
            if (!ok)
                std::cout << "(mensaje de error) No se pueden guardar las secuencias cargadas en "
                          << nombre << " .\n";
            else
                std::cout << "(codificación exitosa) Secuencias codificadas y almacenadas en "
                          << nombre << " .\n";
        }
      

        else if (cmd == "salir" || cmd == "exit" || cmd == "quit") {
            break;
        }

        else {
            std::cout << "Comando no reconocido.\n";
            mostrarMenu();
        }
        
    
    }
    

    return 0;
}

       