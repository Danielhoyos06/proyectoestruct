#include "Secuencia.h"

// Constructor
Secuencia::Secuencia(const std::string& desc, const std::string& seq)
    : description(desc), data(seq) {}

// Getters
const std::string& Secuencia::getDescription() const {
    return description;
}

const std::string& Secuencia::getData() const {
    return data;
}

// Setters
void Secuencia::setDescription(const std::string& desc) {
    description = desc;
}

void Secuencia::setData(const std::string& seq) {
    data = seq;
}

// Longitud
size_t Secuencia::length() const {
    return data.length();
}
