#include "studenthashtable.h"
#include <QDebug>

StudentHashTable::StudentHashTable(int size)
    : table_size(size), item_count(0) {
    items = new HashItem[table_size];
    // Явно инициализируем все элементы как пустые
    for (int i = 0; i < table_size; ++i) {
        items[i] = HashItem();
    }
}

StudentHashTable::~StudentHashTable() {
    delete[] items;
}

unsigned int StudentHashTable::primaryHash(const QString& key) const {
    unsigned long long sum = 0;
    for (int i = 0; i < key.length(); ++i) {
        sum += key[i].unicode();
    }
    return static_cast<unsigned int>(sum % table_size);
}

unsigned int StudentHashTable::probeHash(unsigned int h0, unsigned int i) const {
    return (h0 + i) % static_cast<unsigned int>(table_size);
}

bool StudentHashTable::insert(QString key, int index, QString* errorReason) {
    if (item_count >= table_size) {
        if (errorReason) *errorReason = "full";
        return false;
    }
    unsigned int h0 = primaryHash(key);
    qDebug() << "Добавление ключа" << key << "с индексом" << index << "h0 =" << h0;
    for (unsigned int i = 0; i < table_size; ++i) {
        unsigned int hashIdx = probeHash(h0, i);
        qDebug() << "Попытка" << i << "индекс" << hashIdx << "статус" << static_cast<int>(items[hashIdx].status);
        if (items[hashIdx].status == SlotStatus::EMPTY) {
            items[hashIdx] = HashItem(key, index, SlotStatus::OCCUPIED);
            item_count++;
            qDebug() << "Успешно добавлено в позицию" << hashIdx;
            return true;
        } else if (items[hashIdx].key == key) {
            qDebug() << "Дубликат ключа" << key;
            if (errorReason) *errorReason = "duplicate";
            return false;
        }
    }
    qDebug() << "Таблица заполнена";
    if (errorReason) *errorReason = "full";
    return false;
}

QPair<int, int> StudentHashTable::find(QString key) const {
    unsigned int h0 = primaryHash(key);
    for (unsigned int i = 0; i < table_size; ++i) {
        unsigned int idx = probeHash(h0, i);
        if (items[idx].status == SlotStatus::OCCUPIED && items[idx].key == key) {
            return qMakePair(items[idx].index, i + 1);
        }
        if (items[idx].status == SlotStatus::EMPTY || items[idx].status == SlotStatus::DELETED) break;
    }
    return qMakePair(-1, -1);
}

bool StudentHashTable::remove(QString key) {
    unsigned int h0 = primaryHash(key);
    for (unsigned int i = 0; i < table_size; ++i) {
        unsigned int idx = probeHash(h0, i);
        if (items[idx].status == SlotStatus::OCCUPIED && items[idx].key == key) {
            items[idx] = HashItem(key, idx, SlotStatus::DELETED);
            item_count--;
            return true;
        }
    }
    return false;
}

bool StudentHashTable::isEmpty() const {
    return item_count == 0;
}

int StudentHashTable::size() const { 
    return table_size; 
} 
