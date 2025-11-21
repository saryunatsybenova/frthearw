#ifndef LIST_H
#define LIST_H

#include <functional>
#include <QDate>

class List {
public:
    struct Node {
        int gradeIndex;
        Node* next;
        Node(int idx) : gradeIndex(idx), next(nullptr) {}
    };
    List();
    ~List();
    bool add(int gradeIndex);
    bool remove(int gradeIndex);
    Node* find(int gradeIndex) const;
    Node* find(int gradeIndex, int& steps) const;
    bool isEmpty() const;
    int count() const;
    void forEach(std::function<void(int&)> func);
    
    Node* getTail() const { return tail; }
    Node* getHead() const { return tail ? tail->next : nullptr; }
    
private:
    Node* tail;
    int gradeCount;
};

#endif 
