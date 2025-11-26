#include "Fasta.h"
#include "Huffarbol.h"

#include <fstream>
#include <limits>
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <cstdint>


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
bool Fasta::codificarHuffman(const std::string& nombre_archivo) const {
    if (secuencias_.empty()) {
        // El main imprimirá "(no hay secuencias cargadas) ..."
        return false;
    }

    // 1) Construir tabla de frecuencias global
    std::unordered_map<char, uint64_t> freq;
    for (const auto& s : secuencias_) {
        for (unsigned char c : s.getData()) {
            ++freq[static_cast<char>(c)];
        }
    }
    if (freq.empty()) return false;

    // 2) Construir árbol de Huffman
    HuffmanTree ht;
    ht.build(freq);
    const auto& codes = ht.getCodes();
    if (codes.empty()) return false;

    // 3) Abrir archivo binario de salida
    std::ofstream out(nombre_archivo, std::ios::binary);
    if (!out) return false;

    // 4) Escribir la tabla de símbolos y frecuencias
    //    n: número de símbolos distintos (2 bytes)
    std::vector<std::pair<char, uint64_t>> symbols(freq.begin(), freq.end());
    std::sort(symbols.begin(), symbols.end(),
              [](const auto& a, const auto& b){ return a.first < b.first; });

    uint16_t n_symbols = static_cast<uint16_t>(symbols.size());
    out.write(reinterpret_cast<const char*>(&n_symbols), sizeof(n_symbols));

    // Cada entrada: 1 byte de símbolo + 8 bytes de frecuencia (uint64_t)
    for (const auto& [ch, f] : symbols) {
        out.write(reinterpret_cast<const char*>(&ch), 1);
        out.write(reinterpret_cast<const char*>(&f), sizeof(f));
    }

    // 5) Número de secuencias (4 bytes)
    uint32_t n_seq = static_cast<uint32_t>(secuencias_.size());
    out.write(reinterpret_cast<const char*>(&n_seq), sizeof(n_seq));

    // 6) Para cada secuencia: nombre, longitud, ancho de línea y datos codificados
    for (size_t i = 0; i < secuencias_.size(); ++i) {
        const auto& seq  = secuencias_[i];
        const std::string& nombre = seq.getDescription();
        const std::string& data   = seq.getData();

        // ℓ: longitud del nombre (2 bytes)
        uint16_t name_len = static_cast<uint16_t>(nombre.size());
        out.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        out.write(nombre.data(), name_len);

        // u: longitud de la secuencia en bases (8 bytes)
        uint64_t len_bases = static_cast<uint64_t>(data.size());
        out.write(reinterpret_cast<const char*>(&len_bases), sizeof(len_bases));

        // z: ancho de línea (2 bytes)
        uint16_t width = 60;
        if (i < lineWidths_.size() && lineWidths_[i] > 0) {
            width = lineWidths_[i];
        }
        out.write(reinterpret_cast<const char*>(&width), sizeof(width));

        // Codificar la secuencia completa en bits usando la tabla de Huffman
        std::string bits;
        bits.reserve(data.size() * 4); // tamaño aproximado
        for (char c : data) {
            bits += codes.at(c); // 'at' es seguro: todos los símbolos están en la tabla
        }

        // Rellenar con '0' hasta múltiplo de 8
        size_t padding = (8 - (bits.size() % 8)) % 8;
        bits.append(padding, '0');

        // Convertir cada bloque de 8 bits en un byte
        for (size_t pos = 0; pos < bits.size(); pos += 8) {
            uint8_t byte = 0;
            for (int k = 0; k < 8; ++k) {
                if (bits[pos + k] == '1') {
                    byte |= static_cast<uint8_t>(1u << (7 - k));
                }
            }
            out.write(reinterpret_cast<const char*>(&byte), 1);
        }
    }

    bool ok = out.good();
    out.close();
    return ok;
}

