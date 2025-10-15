#ifndef SECUENCIA_H
#define SECUENCIA_H

#include <string>

class Secuencia {
private:
    std::string description;  // Texto después del símbolo '>'
    std::string data;         // Cadena genética (A, T, G, C, U, etc.)

public:
    // Constructores
    Sequence() = default;
    Sequence(const std::string& desc, const std::string& seq);

    // Getters
    const std::string& getDescription() const;
    const std::string& getData() const;

    // Setters
    void setDescription(const std::string& desc);
    void setData(const std::string& seq);

    // Utilidad: longitud de la secuencia genética
    size_t length() const;
};

#endif // SEQUENCE_H
