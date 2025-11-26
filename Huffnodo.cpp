// HuffmanNode.cpp
#include "Huffnodo.h"

HuffmanNode::HuffmanNode(char s, uint64_t f)
    : symbol(s), frequency(f), left(nullptr), right(nullptr) {}

HuffmanNode::HuffmanNode(HuffmanNode* l, HuffmanNode* r)
    : symbol(0), frequency(l->frequency + r->frequency), left(l), right(r) {}

bool HuffmanNode::isLeaf() const {
    return left == nullptr && right == nullptr;
}