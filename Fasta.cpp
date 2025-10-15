#include "Fasta.h"
#include <fstream>
#include <limits>
#include <cctype>

namespace {
    // Elimina espacios en blanco finales
    inline std::string rstrip(const std::string& s) {
        size_t end = s.size();
        while (end > 0 && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(0, end);
    }
}

size_t Fasta::cargar(const std::string& nombre_archivo) {
    std::ifstream in(nombre_archivo);
    if (!in) {
        // Error de lectura o inexistente
        return std::numeric_limits<size_t>::max();
    }

    std::vector<Secuencia> temp;
    std::string line;
    bool dentro = false;
    std::string desc, datos;

    while (std::getline(in, line)) {
        if (!line.empty() && line[0] == '>') {
            if (dentro) {
                temp.emplace_back(desc, datos);
                datos.clear();
            }
            desc = rstrip(line.substr(1));
            dentro = true;
        } else if (dentro) {
            datos += rstrip(line);
        }
    }

    if (dentro) {
        temp.emplace_back(desc, datos);
    }

    if (temp.empty()) {
        return 0; // No hay secuencias con '>'
    }

    sequences_.swap(temp); // Sobrescribe memoria con las nuevas
    return sequences_.size();
}
