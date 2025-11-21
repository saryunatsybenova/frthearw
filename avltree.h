#ifndef AVLTREE_H
#define AVLTREE_H
#include <functional>

template<typename Key, typename Value, typename Compare = std::less<Key>>
class AVLTree {
public:
    struct Node {
        Key key;
        Value value;
        Node* left;
        Node* right;
        int balanceFactor;
        Node(const Key& k, const Value& v)
            : key(k), value(v), left(nullptr), right(nullptr), balanceFactor(0) {}
    };

    AVLTree() : root(nullptr) {}
    ~AVLTree() { freeTree(root); }

    Node* insert(const Key& key, const Value& value) {
        bool heightIncreased = false;
        Node* inserted = nullptr;
        root = insert(root, key, value, heightIncreased, &inserted);
        return inserted;
    }

    Node* find(const Key& key) const {
        Node* curr = root;
        while (curr) {
            if (key < curr->key) curr = curr->left;
            else if (curr->key > key) curr = curr->right;
            else return curr;
        }
        return nullptr;
    }

    bool remove(const Key& key) {
        bool heightDecreased = false;
        bool removed = false;
        root = remove(root, key, heightDecreased, &removed);
        return removed;
    }

    template<typename Func>
    void inOrder(Func func) const {
        inOrder(root, func);
    }

    const Node* getRoot() const { return root; }
    Node* getRoot() { return root; }

private:
    Node* root;
    Compare comp;

    void freeTree(Node* node) {
        if (!node) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node;
    }

    Node* leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        return y;
    }
    Node* rightRotate(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        y->right = x;
        return y;
    }

    Node* balanceLeft(Node* node, bool& heightChanged, bool isInsert = false) {
        if (isInsert) {
            if (node->balanceFactor == 1) {
                node->balanceFactor = 0;
                heightChanged = false;
            } else if (node->balanceFactor == 0) {
                node->balanceFactor = -1;
            } else {
                Node* leftChild = node->left;
                if (leftChild->balanceFactor == -1) {
                    node = rightRotate(node);
                    node->balanceFactor = 0;
                    node->right->balanceFactor = 0;
                } else {
                    Node* leftRightChild = leftChild->right;
                    node->left = leftRotate(leftChild);
                    node = rightRotate(node);
                    if (leftRightChild->balanceFactor == -1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 1;
                    } else if (leftRightChild->balanceFactor == 1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = -1;
                        node->right->balanceFactor = 0;
                    } else {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 0;
                    }
                }
                heightChanged = false;
            }
        } else {
            if (node->balanceFactor == 1) {
                node->balanceFactor = 0;
                heightChanged = true;
            } else if (node->balanceFactor == 0) {
                node->balanceFactor = -1;
                heightChanged = false;
            } else {
                Node* leftChild = node->left;
                if (leftChild->balanceFactor <= 0) {
                    node = rightRotate(node);
                    if (leftChild->balanceFactor == 0) {
                        node->balanceFactor = 1;
                        node->right->balanceFactor = -1;
                        heightChanged = false;
                    } else {
                        node->balanceFactor = 0;
                        node->right->balanceFactor = 0;
                        heightChanged = true;
                    }
                } else {
                    Node* leftRightChild = leftChild->right;
                    node->left = leftRotate(leftChild);
                    node = rightRotate(node);
                    if (leftRightChild->balanceFactor == -1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 1;
                    } else if (leftRightChild->balanceFactor == 1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = -1;
                        node->right->balanceFactor = 0;
                    } else {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 0;
                    }
                    heightChanged = true;
                }
            }
        }
        return node;
    }

    Node* balanceRight(Node* node, bool& heightChanged, bool isInsert = false) {
        if (isInsert) {
            if (node->balanceFactor == -1) {
                node->balanceFactor = 0;
                heightChanged = false;
            } else if (node->balanceFactor == 0) {
                node->balanceFactor = 1;
            } else {
                Node* rightChild = node->right;
                if (rightChild->balanceFactor == 1) {
                    node = leftRotate(node);
                    node->balanceFactor = 0;
                    node->left->balanceFactor = 0;
                } else {
                    Node* rightLeftChild = rightChild->left;
                    node->right = rightRotate(rightChild);
                    node = leftRotate(node);
                    if (rightLeftChild->balanceFactor == 1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = -1;
                        node->right->balanceFactor = 0;
                    } else if (rightLeftChild->balanceFactor == -1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 1;
                    } else {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 0;
                    }
                }
                heightChanged = false;
            }
        } else {
            if (node->balanceFactor == -1) {
                node->balanceFactor = 0;
                heightChanged = true;
            } else if (node->balanceFactor == 0) {
                node->balanceFactor = 1;
                heightChanged = false;
            } else {
                Node* rightChild = node->right;
                if (rightChild->balanceFactor >= 0) {
                    node = leftRotate(node);
                    if (rightChild->balanceFactor == 0) {
                        node->balanceFactor = -1;
                        node->left->balanceFactor = 1;
                        heightChanged = false;
                    } else {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        heightChanged = true;
                    }
                } else {
                    Node* rightLeftChild = rightChild->left;
                    node->right = rightRotate(rightChild);
                    node = leftRotate(node);
                    if (rightLeftChild->balanceFactor == 1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = -1;
                        node->right->balanceFactor = 0;
                    } else if (rightLeftChild->balanceFactor == -1) {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 1;
                    } else {
                        node->balanceFactor = 0;
                        node->left->balanceFactor = 0;
                        node->right->balanceFactor = 0;
                    }
                    heightChanged = true;
                }
            }
        }
        return node;
    }

    Node* findMax(Node* node) {
        return node->right ? findMax(node->right) : node;
    }

    Node* insert(Node* node, const Key& key, const Value& value, bool& heightIncreased, Node** inserted) {
        if (!node) {
            heightIncreased = true;
            *inserted = new Node(key, value);
            return *inserted;
        }
        if (comp(key, node->key)) {
            node->left = insert(node->left, key, value, heightIncreased, inserted);
            if (heightIncreased) {
                node = balanceLeft(node, heightIncreased, true);
            }
        } else if (comp(node->key, key)) {
            node->right = insert(node->right, key, value, heightIncreased, inserted);
            if (heightIncreased) {
                node = balanceRight(node, heightIncreased, true);
            }
        } else {
            node->value = value;
            *inserted = node;
        }
        return node;
    }

    Node* remove(Node* node, const Key& key, bool& heightDecreased, bool* removed) {
        if (!node) {
            heightDecreased = false;
            *removed = false;
            return nullptr;
        }
        if (comp(key, node->key)) {
            node->left = remove(node->left, key, heightDecreased, removed);
            if (heightDecreased) {
                node = balanceRight(node, heightDecreased, false);
            }
        } else if (comp(node->key, key)) {
            node->right = remove(node->right, key, heightDecreased, removed);
            if (heightDecreased) {
                node = balanceLeft(node, heightDecreased, false);
            }
        } else {
            *removed = true;
            if (!node->left) {
                Node* temp = node->right;
                delete node;
                heightDecreased = true;
                return temp;
            } else if (!node->right) {
                Node* temp = node->left;
                delete node;
                heightDecreased = true;
                return temp;
            } else {
                Node* maxNode = findMax(node->left);
                node->key = maxNode->key;
                node->value = maxNode->value;
                node->left = remove(node->left, maxNode->key, heightDecreased, removed);
                if (heightDecreased) {
                    node = balanceRight(node, heightDecreased, false);
                }
            }
        }
        return node;
    }

    template<typename Func>
    void inOrder(Node* node, Func func) const {
        if (!node) return;
        inOrder(node->left, func);
        func(node->key, node->value);
        inOrder(node->right, func);
    }
};

#endif 
