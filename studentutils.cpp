#include "studentutils.h"
#include "validator.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

InsertStats StudentUtils::insertFromList(const Student* Students, int count, StudentHashTable& hashTable) {
    InsertStats stats;
    
    for (int i = 0; i < count; ++i) {
        const Student& Student = Students[i];
        if (hashTable.item_count >= hashTable.table_size) {
            stats.overflow++;
            continue;
        }
        
        QString errorReason;
        bool inserted = hashTable.insert(Student.name, i, &errorReason);
        if (inserted) {
            stats.inserted++;
        } else if (errorReason == "duplicate") {
            stats.duplicates++;
        } else {
            stats.overflow++;
        }
    }
    
    return stats;
}

void StudentUtils::parseFile(const QString& filename, QStringList& errorLines, int& invalidCount, Student* result, int& resultCount) {
    resultCount = 0;
    invalidCount = 0;
    errorLines.clear();
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorLines << "Не удалось открыть файл " + filename;
        return;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        
        QStringList parts = line.split(" ", Qt::SkipEmptyParts);
        if (parts.size() < 3) {
            invalidCount++;
            errorLines << QString("%1 — недостаточно данных").arg(line);
            continue;
        }
        
        bool okKlass;
        int klass = parts[1].toInt(&okKlass);
        QDate bday = QDate::fromString(parts[2], "dd.MM.yyyy"); // "31.12.2023"
        Student r(parts[0], klass, bday);
        
        QString errorMessage;
        if (Validator::validateStudent(r, errorMessage)) {
            if (result && resultCount < 10000) {
                result[resultCount] = r;
            }
            resultCount++;
        } else {
            QString debugMsg = QString("%1 — %2").arg(line, errorMessage);
            errorLines << debugMsg;
            invalidCount++;
        }
    }
    file.close();
}

bool StudentUtils::insertStudent(const Student& Student, StudentHashTable& hashTable, StudentStorage& storage, QString* errorReason) {
    int idx = -1;
    if (storage.studentCount < MAX_STUDENT) {
        idx = storage.studentCount++;
        storage.student[idx] = Student;
    } else {
        if (errorReason) *errorReason = "storage_full";
        return false;
    }
    bool inserted = hashTable.insert(Student.name, idx, errorReason);
    if (!inserted) {
        storage.studentCount--;
    }
    return inserted;
}

QPair<Student, int> StudentUtils::findStudent(QString fio, const StudentHashTable& hashTable, const StudentStorage& storage) {
    auto result = hashTable.find(fio);
    if (result.first != -1) {
        return qMakePair(storage.student[result.first], result.second);
    }
    return qMakePair(Student(), -1);
}

bool StudentUtils::removeStudent(QString fio, StudentHashTable& hashTable, StudentStorage& storage) {
    auto result = hashTable.find(fio);
    if (result.first == -1) {
        return false;
    }
    int removeIdx = result.first;
    if (!hashTable.remove(fio)) {
        return false;
    }
    int lastIdx = storage.studentCount - 1;
    if (removeIdx != lastIdx) {
        storage.student[removeIdx] = storage.student[lastIdx];
        for (int i = 0; i < hashTable.table_size; ++i) {
            if (hashTable.items[i].status == SlotStatus::OCCUPIED && hashTable.items[i].index == lastIdx) {
                hashTable.items[i].index = removeIdx;
            }
        }
    }
    storage.studentCount--;
    
    return true;
}

QStringList StudentUtils::getHashChainDebug(const StudentHashTable* table, QString fio, const StudentStorage& storage) {
    QStringList lines;
    int h0 = table->primaryHash(fio);
    lines << QString("первичный хеш: h0 = %1").arg(h0);
    int realIdx = -1;
    bool emptyFound = false;
    for (int i = 0; i < table->size(); ++i) {
        int idx = table->probeHash(h0, i);
        if (table->items[idx].status == SlotStatus::OCCUPIED && table->items[idx].key == fio) {
            realIdx = idx;
            break;
        }
        if (table->items[idx].status == SlotStatus::EMPTY) {
            emptyFound = true;
            break;
        }
    }
    if (emptyFound || realIdx == h0) return lines;
    for (int i = 1; i < table->size(); ++i) {
        int idx = table->probeHash(h0, i);
        if (i == 1)
            lines << QString("вторичный хеш: h1 = (h0 + 1) %% %1 = %2").arg(table->size()).arg(idx);
        else
            lines << QString("вторичный хеш (попытка %1): h%2 = (h0 + %1) %% %3 = %4").arg(i).arg(i).arg(table->size()).arg(idx);
        if (idx == realIdx) break;
        if (table->items[idx].status == SlotStatus::EMPTY) break;
    }
    return lines;
} 