#ifndef FASTA_H
#define FASTA_H

#include <string>
#include <vector>
#include "Secuencia.h"

class Fasta {
private:
    std::vector<Secuencia> secuencias_; // Estructura lineal para almacenar las secuencias

public:
    // Carga un archivo FASTA y devuelve:
    // - SIZE_MAX → error de E/S (no se puede leer)
    // - 0        → archivo vacío (sin '>')
    // - n >= 1   → n secuencias cargadas
    size_t cargar(const std::string& nombre_archivo);
    const std::vector<Secuencia>& secuencias() const { return secuencias_; }
    size_t contarSubsecuencia(const std::string& subseq) const;
};

#endif // FASTADB_H
