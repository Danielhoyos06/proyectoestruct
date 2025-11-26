#ifndef FASTA_H
#define FASTA_H

#include <string>
#include <vector>
#include "Secuencia.h"
#include <unordered_map>
#include <cstdint>
#include "Huffarbol.h"

class Fasta {
private:
    std::vector<Secuencia> secuencias_; // Estructura lineal para almacenar las secuencias
    std::vector<size_t> lineWidths_; 

public:
    
    size_t cargar(const std::string& nombre_archivo); //funcion para cargar un archivo 
    bool guardar(const std::string& nombre_archivo) const;//funcion para guardar en un archivo las secuencisa con la respetiva subseq enmmascarada
    const std::vector<Secuencia>& secuencias() const { return secuencias_; } //funcion para listar
    size_t contarSubsecuencia(const std::string& subseq) const; //funcion para hallar si una subseq dada por el usuario existe 
    std::unordered_map<char, size_t> obtenerHistograma(const std::string& descripcion) const;//funcion para generar el histograma
    size_t enmascararSubsecuencia(const std::string& subseq);//funcion para enmascarar una subseq dada por el usuario
    bool codificarHuffman(const std::string& nombre_archivo) const;
    //bool decodificarHuffman(const std::string& nombre_archivo);
};

#endif // FASTADB_H
