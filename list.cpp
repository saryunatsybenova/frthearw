#include "list.h"

List::List() : tail(nullptr), gradeCount(0) {}

List::~List() {
    if (!tail) return;
    Node* head = tail->next;
    Node* cur = head;
    do {
        Node* next = cur->next;
        delete cur;
        cur = next;
    } while (cur != head);
    tail = nullptr;
    gradeCount = 0;
}

bool List::add(int gradeIndex) {
    if (!isEmpty()) {
        Node* head = tail->next;
        Node* cur = head;
        do {
            if (cur->gradeIndex == gradeIndex) {
                return false;
            }
            cur = cur->next;
        } while (cur != head);
    }
    Node* newNode = new Node(gradeIndex);
    if (!tail) {
        tail = newNode;
        tail->next = tail;
    } else {
        newNode->next = tail->next;
        tail->next = newNode;
        tail = newNode;
    }
    gradeCount++;
    return true;
}

bool List::remove(int gradeIndex) {
    if (isEmpty()) return false;
    Node* head = tail->next;
    Node* prev = tail;
    Node* cur = head;
    do {
        if (cur->gradeIndex == gradeIndex) {
            if (cur == tail && cur == head) {
                delete cur;
                tail = nullptr;
            } else {
                prev->next = cur->next;
                if (cur == tail) tail = prev;
                if (cur == head) tail->next = cur->next;
                delete cur;
            }
            gradeCount--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    } while (cur != head);
    return false;
}

List::Node* List::find(int gradeIndex) const {
    if (isEmpty()) return nullptr;
    Node* head = tail->next;
    Node* cur = head;
    do {
        if (cur->gradeIndex == gradeIndex) return cur;
        cur = cur->next;
    } while (cur != head);
    return nullptr;
}

List::Node* List::find(int gradeIndex, int& steps) const {
    if (isEmpty()) return nullptr;
    Node* head = tail->next;
    Node* cur = head;
    do {
        steps++;
        if (cur->gradeIndex == gradeIndex) return cur;
        cur = cur->next;
    } while (cur != head);
    return nullptr;
}

bool List::isEmpty() const {
    return tail == nullptr;
}

int List::count() const {
    return gradeCount;
}

void List::forEach(std::function<void(int&)> func) {
    if (isEmpty()) return;
    Node* head = tail->next;
    Node* cur = head;
    do {
        func(cur->gradeIndex);
        cur = cur->next;
    } while (cur != head);
} 
