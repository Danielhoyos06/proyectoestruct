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
bool Fasta::decodificarHuffman(const std::string& nombre_archivo) {
    std::ifstream in(nombre_archivo, std::ios::binary);
    if (!in) return false;

    // 1) Leer número de símbolos (tabla Huffman)
    uint16_t n_symbols = 0;
    if (!in.read(reinterpret_cast<char*>(&n_symbols), sizeof(n_symbols))) return false;
    if (n_symbols == 0) return false;

    // 2) Leer pares (símbolo, frecuencia)
    std::unordered_map<char, uint64_t> freq;
    for (uint16_t i = 0; i < n_symbols; ++i) {
        char ch;
        uint64_t f;
        if (!in.read(reinterpret_cast<char*>(&ch), 1)) return false;
        if (!in.read(reinterpret_cast<char*>(&f), sizeof(f))) return false;
        freq[ch] = f;
    }

    // 3) Reconstruir árbol de Huffman
    HuffmanTree ht;
    ht.build(freq);
    HuffmanNode* root = ht.getRoot();
    if (!root || ht.getCodes().empty()) return false;

    // 4) Leer número de secuencias
    uint32_t n_seq = 0;
    if (!in.read(reinterpret_cast<char*>(&n_seq), sizeof(n_seq))) return false;

    // Nuevos contenedores (mismos tipos que en Fasta.h)
    std::vector<Secuencia> nuevasSecuencias;
    std::vector<size_t>    nuevosWidths;
    nuevasSecuencias.reserve(n_seq);
    nuevosWidths.reserve(n_seq);

    // --- Lector de bits sobre el archivo ---
    uint8_t currentByte = 0;
    int bitsLeft = 0;  // cuántos bits quedan en currentByte

    auto readBit = [&]() -> int {
        if (bitsLeft == 0) {
            if (!in.read(reinterpret_cast<char*>(&currentByte), 1)) {
                return -1; // EOF o error
            }
            bitsLeft = 8;
        }
        int bit = (currentByte >> 7) & 1; // bit más significativo
        currentByte <<= 1;
        --bitsLeft;
        return bit;
    };

    // 5) Procesar cada secuencia
    for (uint32_t si = 0; si < n_seq; ++si) {
        // 5.1 Longitud del nombre
        uint16_t name_len = 0;
        if (!in.read(reinterpret_cast<char*>(&name_len), sizeof(name_len))) return false;

        std::string nombre(name_len, '\0');
        if (!in.read(&nombre[0], name_len)) return false;

        // 5.2 Longitud de la secuencia en bases
        uint64_t len_bases = 0;
        if (!in.read(reinterpret_cast<char*>(&len_bases), sizeof(len_bases))) return false;

        // 5.3 Ancho de línea original
        uint16_t width16 = 0;
        if (!in.read(reinterpret_cast<char*>(&width16), sizeof(width16))) return false;
        size_t width = static_cast<size_t>(width16);

        // 5.4 Decodificar exactamente len_bases símbolos usando el árbol
        std::string decoded;
        decoded.reserve(static_cast<size_t>(len_bases));

        for (uint64_t k = 0; k < len_bases; ++k) {
            HuffmanNode* node = root;
            while (node && !node->isLeaf()) {
                int bit = readBit();
                if (bit < 0) return false; // archivo truncado

                node = (bit == 0) ? node->left : node->right;
            }
            if (!node) return false; // árbol inconsistent / bits corruptos

            decoded.push_back(node->symbol);
        }

        // Descartar los bits de padding que quedaron en el último byte
        bitsLeft = 0;

        nuevasSecuencias.emplace_back(nombre, decoded);
        nuevosWidths.push_back(width);
    }

    // 6) Reemplazar el contenido en memoria solo si TODO salió bien
    secuencias_.swap(nuevasSecuencias);
    lineWidths_.swap(nuevosWidths);

    return true;
}
