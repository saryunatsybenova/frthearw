#ifndef STUDENTUTILS_H
#define STUDENTUTILS_H

#include <QString>
#include <QStringList>
#include "student.h"
#include "studenthashtable.h"

class StudentUtils {
public:
    static InsertStats insertFromList(const Student* Students, int count, StudentHashTable& hashTable);
    static void parseFile(const QString& filename, QStringList& errorLines, int& invalidCount, Student* result, int& resultCount);

    static bool insertStudent(const Student& Student, StudentHashTable& hashTable, StudentStorage& storage, QString* errorReason = nullptr);
    static QPair<Student, int> findStudent(QString fio, const StudentHashTable& hashTable, const StudentStorage& storage);
    static bool removeStudent(QString fio, StudentHashTable& hashTable, StudentStorage& storage);

    static QStringList getHashChainDebug(const StudentHashTable* table, QString fio, const StudentStorage& storage);
};

#endif 