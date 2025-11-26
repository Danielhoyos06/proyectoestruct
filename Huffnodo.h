// HuffmanNode.h
#ifndef HUFFNODO_H
#define HUFFNODO_H

#include <cstdint>

class HuffmanNode {
public:
    char symbol;          // carácter (solo válido si es hoja)
    uint64_t frequency;   // frecuencia del símbolo o suma de subárboles

    HuffmanNode* left;
    HuffmanNode* right;

    // Construye una hoja
    HuffmanNode(char s, uint64_t f);

    // Construye un nodo interno a partir de dos hijos
    HuffmanNode(HuffmanNode* l, HuffmanNode* r);

    bool isLeaf() const;
};

#endif