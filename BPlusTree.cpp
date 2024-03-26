#include "BPlusTree.h"
#include <queue>
using namespace std;

BPlusNode::BPlusNode() {
    for (int i = 0; i < DEGREE + 1; ++i) {
        children[i] = nullptr;
    }
}

BPlusNode::~BPlusNode() {
    for (int i = 0; i < DEGREE + 1; ++i) {
        delete children[i];
    }
}

BPlusTree::BPlusTree() : root(nullptr) {}

BPlusTree::~BPlusTree() {
    delete root;
}

void BPlusTree::insert(int key, int value) {
    if (root == nullptr) {
        root = new BPlusNode();
        root->keys.push_back(key);
        root->values.push_back(value);
        root->isLeaf = true;
        return;
    }

    BPlusNode* leafNode = findLeafNode(key);
    insertInternal(key, leafNode, nullptr);
}

void BPlusTree::insertInternal(int key, BPlusNode* cursor, BPlusNode* child) {
    if (cursor->keys.size() < DEGREE - 1) {
        int index = 0;
        while (index < cursor->keys.size() && cursor->keys[index] < key) {
            index++;
        }
        cursor->keys.insert(cursor->keys.begin() + index, key);
        cursor->values.insert(cursor->values.begin() + index, key);
        if (child != nullptr) {
            cursor->children[index + 1] = child;
        }
        return;
    }
    
    // Splitting the node
    vector<int> tempKeys(cursor->keys.begin(), cursor->keys.end());
    vector<int> tempValues(cursor->values.begin(), cursor->values.end());
    vector<BPlusNode*> tempChildren(cursor->children, cursor->children + DEGREE);
    int index = 0;
    while (index < tempKeys.size() && tempKeys[index] < key) {
        index++;
    }
    tempKeys.insert(tempKeys.begin() + index, key);
    tempValues.insert(tempValues.begin() + index, key);
    if (child != nullptr) {
        tempChildren[index + 1] = child;
    }

    cursor->keys.assign(tempKeys.begin(), tempKeys.begin() + DEGREE / 2);
    cursor->values.assign(tempValues.begin(), tempValues.begin() + DEGREE / 2);
    cursor->isLeaf = false;
    for (int i = 0; i < DEGREE / 2; ++i) {
        cursor->children[i] = tempChildren[i];
    }

    BPlusNode* newChild = new BPlusNode();
    newChild->keys.assign(tempKeys.begin() + DEGREE / 2, tempKeys.end());
    newChild->values.assign(tempValues.begin() + DEGREE / 2, tempValues.end());
    newChild->isLeaf = false;
    for (int i = DEGREE / 2; i < DEGREE + 1; ++i) {
        newChild->children[i - DEGREE / 2] = tempChildren[i];
    }

    int splitKey = newChild->keys[0];

    if (cursor == root) {
        root = new BPlusNode();
        root->keys.push_back(splitKey);
        root->values.push_back(splitKey);
        root->children[0] = cursor;
        root->children[1] = newChild;
        root->isLeaf = false;
        return;
    }

    insertInternal(splitKey, findParent(root, cursor), newChild);
}

// Function to find the parent node of a given node
BPlusNode* BPlusTree::findParent(BPlusNode* current, BPlusNode* child) {
    if (root == nullptr || root == child) {
        return nullptr; // No parent for the root or child is the root
    }

    // Iterate through the children of the current node to find the child
    for (int i = 0; i < DEGREE; ++i) {
        if (current->children[i] == child) {
            return current; // Parent found
        }
    }
    // Recursively search for the parent in the children nodes
    for (int i = 0; i < DEGREE; ++i) {
        BPlusNode* parent = findParent(current->children[i], child);
        if (parent != nullptr) {
            return parent; // Parent found in the subtree
        }
    }
    return nullptr; // Parent not found
}

void BPlusTree::remove(int key) {
    if (root == nullptr) {
        cout << "Tree is empty" << endl;
        return;
    }
    removeInternal(root, key);
}

// Function to remove an internal key from the B+ tree
void BPlusTree::removeInternal(BPlusNode* node, int key) {
    // Find the index of the key to be removed
    int index = 0;
    while (index < node->keys.size() && key > node->keys[index]) {
        ++index;
    }

    // Recursively remove the key from the child node
    if (node->isLeaf) {
        // Handle leaf node removal
    } else {
        removeInternal(node->children[index], key);
    }
}

void BPlusTree::redistributeKeys(BPlusNode* parent, int index) {}

void BPlusTree::mergeNodes(BPlusNode* parent, int index) {}

void BPlusTree::printTree() {
    if (root == nullptr) {
        cout << "Empty tree" << endl;
        return;
    }

    queue<BPlusNode*> q;
    q.push(root);
    while (!q.empty()) {
        int levelSize = q.size();
        for (int i = 0; i < levelSize; ++i) {
            BPlusNode* node = q.front();
            q.pop();
            cout << "[ ";
            for (int j = 0; j < node->keys.size(); ++j) {
                cout << node->keys[j] << " ";
            }
            cout << "] ";
            if (!node->isLeaf) {
                for (int j = 0; j <= node->keys.size(); ++j) {
                    q.push(node->children[j]);
                }
            }
        }
        cout << endl;
    }
}

vector<int> BPlusTree::inOrderTraversalHelper(BPlusNode* node) {
    vector<int> result;
    if (node != nullptr) {
        if (node->isLeaf) {
            for (int i = 0; i < node->keys.size(); ++i) {
                result.push_back(node->keys[i]);
            }
        } else {
            for (int i = 0; i < node->keys.size(); ++i) {
                vector<int> subtreeResult = inOrderTraversalHelper(node->children[i]);
                for (int j = 0; j < subtreeResult.size(); ++j) {
                    result.push_back(subtreeResult[j]);
                }
                result.push_back(node->keys[i]);
            }
            vector<int> subtreeResult = inOrderTraversalHelper(node->children[node->keys.size()]);
            for (int j = 0; j < subtreeResult.size(); ++j) {
                result.push_back(subtreeResult[j]);
            }
        }
    }
    return result;
}

vector<int> BPlusTree::inOrderTraversal() {
    return inOrderTraversalHelper(root);
}

int BPlusTree::findKey(int key) {
    BPlusNode* leaf = findLeafNode(key);
    if (leaf == nullptr) {
        return -1; // Tree is empty or key not found
    }

    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            return key; // Key found
        }
    }

    return -1; // Key not found
}

// Function to find the leaf node containing the given key
BPlusNode* BPlusTree::findLeafNode(int key) {
    if (root == nullptr) {
        return nullptr; // Tree is empty
    }

    BPlusNode* current = root;
    while (!current->isLeaf) {
        // Find the child node to traverse based on the key
        int index = 0;
        while (index < current->keys.size() && key >= current->keys[index]) {
            index++;
        }
        current = current->children[index];
    }
    return current;
}

int BPlusTree::findMinKey() {
    if (root == nullptr) {
        return -1;
    }
    BPlusNode* current = root;
    while (!current->isLeaf) {
        current = current->children[0];
    }
    return current->keys[0];
}

// Function to find the maximum key in the B+ tree
int BPlusTree::findMaxKey() {
    if (root == nullptr) {
        return -1;
    }
    BPlusNode* current = root;
    while (!current->isLeaf) {
        current = current->children[current->keys.size()];
    }
    return current->keys[current->keys.size() - 1];
}

void BPlusTree::clearTree(BPlusNode* node) {
    if (node == nullptr) {
        return;
    }
    if (!node->isLeaf) {
        for (int i = 0; i < node->keys.size(); ++i) {
            clearTree(node->children[i]);
        }
    }
    delete node;
}

void BPlusTree::clearTree() {
    clearTree(root);
    root = nullptr;
}

void BPlusTree::insertIntoLeaf(BPlusNode* leaf, int key, int value) {
    // Find the position to insert the key in the leaf node
    auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);

    // Insert the key and value at the correct position
    int index = it - leaf->keys.begin();
    leaf->keys.insert(it, key);
    leaf->values.insert(leaf->values.begin() + index, value);
}

void BPlusTree::splitLeafNode(BPlusNode* leaf) {
    // Create a new leaf node
    BPlusNode* newLeaf = new BPlusNode();

    // Move half of the keys and values from the original leaf node to the new leaf node
    int splitIndex = leaf->keys.size() / 2;
    newLeaf->keys.assign(leaf->keys.begin() + splitIndex, leaf->keys.end());
    newLeaf->values.assign(leaf->values.begin() + splitIndex, leaf->values.end());
    leaf->keys.erase(leaf->keys.begin() + splitIndex, leaf->keys.end());
    leaf->values.erase(leaf->values.begin() + splitIndex, leaf->values.end());

    // Update the leaf node pointers
    newLeaf->isLeaf = true;
    newLeaf->children[DEGREE] = leaf->children[DEGREE];
    leaf->children[DEGREE] = newLeaf;

    // If the split leaf node is not the root, update the parent node
    if (leaf != root) {
        BPlusNode* parent = findParent(root, leaf);
        insertInternal(newLeaf->keys[0], parent, newLeaf);
    } else {
        // If the split leaf node is the root, create a new root
        root = new BPlusNode();
        root->keys.push_back(newLeaf->keys[0]);
        root->values.push_back(newLeaf->keys[0]);
        root->children[0] = leaf;
        root->children[1] = newLeaf;
        root->isLeaf = false;
    }
}
