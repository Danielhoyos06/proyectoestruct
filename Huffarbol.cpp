#include "Huffarbol.h"
#include <queue>

struct HuffmanCmp {
    bool operator()(HuffmanNode* a, HuffmanNode* b) const {
        return a->frequency > b->frequency; // min-heap
    }
};

HuffmanTree::HuffmanTree() : root(nullptr) {}

HuffmanTree::~HuffmanTree() {
    freeTree(root);
}

void HuffmanTree::build(const std::unordered_map<char, uint64_t>& freq) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, HuffmanCmp> pq;

    for (auto& [ch, f] : freq) {
        pq.push(new HuffmanNode(ch, f));
    }

    if (pq.empty()) {
        root = nullptr;
        return;
    }

    while (pq.size() > 1) {
        HuffmanNode* a = pq.top(); pq.pop();
        HuffmanNode* b = pq.top(); pq.pop();
        pq.push(new HuffmanNode(a, b));
    }

    root = pq.top();
    codeTable.clear();
    buildCodes(root, "");
}

void HuffmanTree::buildCodes(HuffmanNode* node, const std::string& prefix) {
    if (!node) return;

    if (node->isLeaf()) {
        codeTable[node->symbol] = prefix.empty() ? "0" : prefix;
        return;
    }

    buildCodes(node->left,  prefix + "0");
    buildCodes(node->right, prefix + "1");
}

const std::unordered_map<char, std::string>& HuffmanTree::getCodes() const {
    return codeTable;
}

std::string HuffmanTree::encode(const std::string& text) const {
    std::string bits;
    for (char c : text) bits += codeTable.at(c);
    return bits;
}

std::string HuffmanTree::decode(const std::string& bits) const {
    std::string out;
    HuffmanNode* node = root;
    for (char b : bits) {
        node = (b == '0') ? node->left : node->right;
        if (node->isLeaf()) {
            out += node->symbol;
            node = root;
        }
    }
    return out;
}

void HuffmanTree::freeTree(HuffmanNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

