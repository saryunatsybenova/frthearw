#include "gradesutils.h"
#include "studentutils.h"
#include <algorithm>

bool DateLess::operator()(const std::string& a, const std::string& b) const {
    int d1 = std::stoi(a.substr(0, 2)), m1 = std::stoi(a.substr(3, 2)), y1 = std::stoi(a.substr(6, 4));
    int d2 = std::stoi(b.substr(0, 2)), m2 = std::stoi(b.substr(3, 2)), y2 = std::stoi(b.substr(6, 4));
    if (y1 != y2) return y1 < y2;
    if (m1 != m2) return m1 < m2;
    return d1 < d2;
}

bool GradeUtils::insertGrade(AVLTree<QString, List*>& tree, int GradeIndex, GradeStorage& storage) {
    const Grade& b = storage.grades[GradeIndex];
    auto node = tree.find(b.FIO);
    if (node) {
        node->value->add(GradeIndex);
        return false; 
    } else {
        auto* list = new List();
        list->add(GradeIndex);
        tree.insert(b.FIO, list);
        return true; 
    }
}

bool GradeUtils::removeGrade(AVLTree<QString, List*>& tree, int GradesIndex, GradeStorage& storage) {
    const Grade& b = storage.grades[GradesIndex];
    auto node = tree.find(b.FIO);
    if (!node || !node->value) return false;
    bool removed = GradeUtils::removeGradeByDate(node->value, b.Mark_date, storage);
    if (node->value->isEmpty()) {
        delete node->value;
        tree.remove(b.FIO);
    }
    return removed;
}

const Grade* GradeUtils::findGrade(const AVLTree<QString, List*>& tree, int GradeIndex, GradeStorage& storage, int& steps) {
    steps = 0;
    const Grade& b = storage.grades[GradeIndex];
    auto node = tree.getRoot();
    while (node) {
        steps++;
        if (b.FIO < node->key) {
            node = node->left;
        } else if (b.FIO > node->key) {
            node = node->right;
        } else {
            if (node->value) {
                auto found = GradeUtils::findGradeByDate(node->value, b.Mark_date, storage, steps);
                if (found) return &storage.grades[found->gradeIndex];
            }
            break;
        }
    }
    return nullptr;
}

void GradeUtils::inOrderGrades(const AVLTree<QString, List*>& tree, int* indices, int& count, GradeStorage& storage) {
    count = 0;
    tree.inOrder([&](const QString& key, List* Grades) {
        if (Grades) {
            Grades->forEach([&](int idx) {
                if (indices) indices[count] = idx;
                count++;
            });
        }
    });
}

bool GradeUtils::isGradesEmpty(const AVLTree<QString, List*>& tree) {
    bool empty = true;
    tree.inOrder([&empty](const QString& key, List*) { empty = false; });
    return empty;
}

void GradeUtils::filterByBirthdayAndSubject(
    const AVLTree<QString, List*>& tree,
        StudentHashTable* studentTable,
        Grade*& result, int& resultCount, int& resultCapacity,
        const QString& date, const QDate& birthday, const QString& subject,
        GradeStorage& storage,
        StudentStorage& storageStudent
) {
    resultCount = 0;
    if (!studentTable) return;
    
    auto node = tree.find(date);
    if (node && node->value) {
        node->value->forEach([&](int idx) {
            const Grade& b = storage.grades[idx];
            bool subjectOk = (b.subject == subject);
            bool birthdayOk = false;
            auto studentInfo = StudentUtils::findStudent(b.FIO, *studentTable, storageStudent);
            if (studentInfo.second != -1) {
                birthdayOk = (studentInfo.first.birth == birthday);
            }
            if (subjectOk && birthdayOk) {
                addGradeToArray(result, resultCount, resultCapacity, b);
            }
        });
    }
}

void GradeUtils::addGradeToArray(Grade*& arr, int& count, int& capacity, const Grade& b) {
    if (count == capacity) {
        int newCapacity = (capacity == 0) ? 8 : capacity * 2;
        Grade* newArr = new Grade[newCapacity];
        for (int i = 0; i < count; ++i)
            newArr[i] = arr[i];
        delete[] arr;
        arr = newArr;
        capacity = newCapacity;
    }
    arr[count++] = b;
}

int GradeUtils::removeAllGradesByFIO(AVLTree<QString, List*>& tree, QString FIO, GradeStorage& storage) {
    auto node = tree.find(FIO);
    if (!node || !node->value) return 0;
    int count = node->value->count();
    if (count == 0) {
        delete node->value;
        tree.remove(FIO);
        return 0;
    }
    int* indices = new int[count];
    int idxCount = 0;
    node->value->forEach([&](int idx) {
        if (idxCount < count) indices[idxCount++] = idx;
    });
    std::sort(indices, indices + idxCount, std::greater<int>());
    int removed = 0;
    for (int i = 0; i < idxCount; ++i) {
        int idx = indices[i];
        if (idx >= 0 && idx < storage.GradesCount) {
            int lastIdx = storage.GradesCount - 1;
            if (idx != lastIdx) {
                Grade moved = storage.grades[lastIdx];
                storage.grades[idx] = moved;
                tree.inOrder([&](const QString& key, List* list) {
                    if (list) {
                        list->forEach([&](int& bidx) {
                            if (bidx == lastIdx) bidx = idx;
                        });
                    }
                });
            }
            storage.GradesCount--;
            removed++;
        }
    }
    delete[] indices;
    delete node->value;
    tree.remove(FIO);
    return removed;
}

void GradeUtils::freeAllGradeLists(AVLTree<QString, List*>& tree) {
    tree.inOrder([&](const QString& key, List* list) {
        delete list;
    });
}

bool GradeUtils::removeGradeByDate(List* list, const QDate& Mark_date, const GradeStorage& storage) {
    if (!list || list->isEmpty()) return false;
    
    List::Node* head = list->getHead();
    List::Node* prev = list->getTail();
    List::Node* cur = head;
    
    do {
        const Grade& b = storage.grades[cur->gradeIndex];
        if (b.Mark_date == Mark_date) {
            return list->remove(cur->gradeIndex);
        }
        prev = cur;
        cur = cur->next;
    } while (cur != head);
    
    return false;
}

List::Node* GradeUtils::findGradeByDate(List* list, const QDate& Mark_date, const GradeStorage& storage) {
    if (!list || list->isEmpty()) return nullptr;
    
    List::Node* head = list->getHead();
    List::Node* cur = head;
    
    do {
        const Grade& b = storage.grades[cur->gradeIndex];
        if (b.Mark_date == Mark_date) return cur;
        cur = cur->next;
    } while (cur != head);
    
    return nullptr;
}

List::Node* GradeUtils::findGradeByDate(List* list, const QDate& Mark_date, const GradeStorage& storage, int& steps) {
    if (!list || list->isEmpty()) return nullptr;
    
    List::Node* head = list->getHead();
    List::Node* cur = head;
    
    do {
        steps++;
        const Grade& b = storage.grades[cur->gradeIndex];
        if (b.Mark_date == Mark_date) return cur;
        cur = cur->next;
    } while (cur != head);
    
    return nullptr;
} 
