#include "Fasta.h"
#include <fstream>
#include <limits>
#include <cctype>
#include <algorithm>


namespace {
    // Elimina espacios en blanco finales
    inline std::string rstrip(const std::string& s) {
        size_t end = s.size();
        while (end > 0 && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(0, end);
    }

    inline size_t contarOcurrencias(const std::string& text, const std::string& needle) {
        if (needle.empty()) return 0;
        size_t count = 0;
        size_t pos = 0;
        while ((pos = text.find(needle, pos)) != std::string::npos) {
            ++count;
            ++pos; // permitir solapamientos
        }
        return count;
    }
        inline void hallarPosicionesrep(const std::string& text,
                                          const std::string& needle,
                                          std::vector<size_t>& pos_out) {
        if (needle.empty()) return;
        size_t pos = 0;
        while ((pos = text.find(needle, pos)) != std::string::npos) {
            pos_out.push_back(pos);
            ++pos; // permitir solapamientos
        }
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

    secuencias_.swap(temp); // Sobrescribe memoria con las nuevas
    return secuencias_.size();
}

bool Fasta::guardar(const std::string& nombre_archivo) const {
    if (secuencias_.empty()) return false;

    std::ofstream out(nombre_archivo);
    if (!out) return false;

    for (size_t i = 0; i < secuencias_.size(); ++i) {
        out << ">" << secuencias_[i].getDescription() << "\n";
        const std::string& data = secuencias_[i].getData();

        size_t width = (i < lineWidths_.size() && lineWidths_[i] > 0)
                        ? lineWidths_[i]
                        : 60; // valor por defecto

        for (size_t pos = 0; pos < data.size(); pos += width) {
            out << data.substr(pos, width) << "\n";
        }
    }

    return true;
}

size_t Fasta::contarSubsecuencia(const std::string& subseq) const {
    size_t total = 0;
    for (const auto& s : secuencias_) {
        total += contarOcurrencias(s.getData(), subseq);
    }
    return total;
}
std::unordered_map<char, size_t> Fasta::obtenerHistograma(const std::string& descripcion) const {
    for (const auto& s : secuencias_) {
        if (s.getDescription() == descripcion) {
            std::unordered_map<char, size_t> h;
            for (unsigned char c : s.getData()) {
                if (c == '\n' || c == '\r' || c == ' ' || c == '\t') continue;
                ++h[static_cast<char>(c)];
            }
            return h;
        }
    }
    return {}; // Secuencia no encontrada
} //para el desarrollo de esta funcion se hizo uso de una herramienta de IA (chatgpt) para generar el esqueleto y explicacion de la funcion 
// con el prompt "elabora el esqueleto de una funcion que genere un histograma apartir de una secuencia de letras"

size_t Fasta::enmascararSubsecuencia(const std::string& subseq) {
    if (subseq.empty()) return 0;

    size_t total = 0;
    const size_t L = subseq.size();

    for (auto& s : secuencias_) {
        const std::string& original = s.getData();
        std::vector<size_t> pos;
        hallarPosicionesrep(original, subseq, pos);

        if (!pos.empty()) {
            std::string masked = original; // trabajamos sobre una copia para no romper la búsqueda
            for (size_t p : pos) {
                for (size_t i = 0; i < L && p + i < masked.size(); ++i) {
                    masked[p + i] = 'X';
                }
            }
            s.setData(masked);
            total += pos.size();
        }
    }

    return total;
}
