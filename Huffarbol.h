#ifndef HUFFARBOL_H
#define HUFFARBOL_H

#include <unordered_map>
#include <queue>
#include <string>
#include <vector>
#include "Huffnodo.h"

class HuffmanTree {
private:
    HuffmanNode* root;
    std::unordered_map<char, std::string> codeTable;

    void buildCodes(HuffmanNode* node, const std::string& prefix);
    void freeTree(HuffmanNode* node);

public:
    HuffmanTree();
    ~HuffmanTree();

    // Construir el árbol a partir de una tabla de frecuencias
    void build(const std::unordered_map<char, uint64_t>& freq);

    // Obtener la tabla de códigos (char -> string de '0' y '1')
    const std::unordered_map<char, std::string>& getCodes() const;

    // Opcionales: helpers para probar
    std::string encode(const std::string& text) const;
    std::string decode(const std::string& bits) const;
    HuffmanNode* getRoot() const { return root; }
};

#endif