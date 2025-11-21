#pragma once
#include "avltree.h"
#include "list.h"
#include "grades.h"
#include "studenthashtable.h"
#include "student.h"
#include <QString>
#include <string>
#include <sstream>

struct DateLess {
    bool operator()(const std::string& a, const std::string& b) const;
};

class GradeUtils {
public:
    static bool insertGrade(AVLTree<QString, List*>& tree, int GradeIndex, GradeStorage& storage);
    static bool removeGrade(AVLTree<QString, List*>& tree, int GradeIndex, GradeStorage& storage);
    static const Grade* findGrade(const AVLTree<QString, List*>& tree, int GradeIndex, GradeStorage& storage, int& steps);
    static void inOrderGrades(const AVLTree<QString, List*>& tree, int* indices, int& count, GradeStorage& storage);
    static bool isGradesEmpty(const AVLTree<QString, List*>& tree);

    static void filterByBirthdayAndSubject(
        const AVLTree<QString, List*>& tree,
        StudentHashTable* studentTable,
        Grade*& result, int& resultCount, int& resultCapacity,
        const QString& date, const QDate& birthday, const QString& subject,
        GradeStorage& storage,
        StudentStorage& storageStudent
    );

    static void addGradeToArray(Grade*& arr, int& count, int& capacity, const Grade& b);
    static int removeAllGradesByFIO(AVLTree<QString, List*>& tree, QString FIO, GradeStorage& storage);

    static void freeAllGradeLists(AVLTree<QString, List*>& tree);
    static void freeAllGradeLists(AVLTree<std::string, List*>& tree);

    static bool removeGradeByDate(List* list, const QDate& Mark_date, const GradeStorage& storage);
    static List::Node* findGradeByDate(List* list, const QDate& Mark_date, const GradeStorage& storage);
    static List::Node* findGradeByDate(List* list, const QDate& Mark_date, const GradeStorage& storage, int& steps);

    template<typename Key, typename Compare, typename Formatter>
    static void printGradesTree(const AVLTree<Key, List*, Compare>& tree, QString& out, GradeStorage& Gradestorage, Formatter formatter) {
        std::function<void(const typename AVLTree<Key, List*, Compare>::Node*, int)> printNode;
        printNode = [&](const typename AVLTree<Key, List*, Compare>::Node* node, int depth) {
            if (!node) return;
            printNode(node->right, depth + 1);
            out += QString(depth * 5, ' ');
            if
            out += node->key + ": ";
            if (node->value) {
                QString GradesStr;
                bool first = true;
                node->value->forEach([&](int idx) {
                    if (!first) GradesStr += " | ";
                    GradesStr += formatter(Gradestorage.grades[idx]);
                    GradesStr += " {" + QString::number(idx) + "}";
                    first = false;
                });
                out += GradesStr;
            }
            out += "\n";
            printNode(node->left, depth + 1);
        };
        printNode(tree.getRoot(), 0);
    }

private:
    GradeUtils() = delete;
    GradeUtils(const GradeUtils&) = delete;
    GradeUtils& operator=(const GradeUtils&) = delete;
}; 
