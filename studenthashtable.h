#ifndef STUDENTHASHTABLE_H
#define STUDENTHASHTABLE_H

#include <QString>
#include <QPair>

enum class SlotStatus : unsigned {
    EMPTY = 0,
    OCCUPIED = 1,
    DELETED = 2
};

struct HashItem {
    QString key;
    int index;
    SlotStatus status;
    
    HashItem() : key(""), index(-1), status(SlotStatus::EMPTY) {}
    HashItem(QString k, int idx, SlotStatus s) : key(k), index(idx), status(s) {}
};

struct InsertStats {
    int inserted = 0;
    int duplicates = 0;
    int overflow = 0;
};

class StudentHashTable {
public:
    StudentHashTable(int size);
    ~StudentHashTable();

    unsigned int primaryHash(const QString &key) const;

    bool insert(QString key, int index, QString* errorReason = nullptr);
    QPair<int, int> find(QString key) const; 
    bool remove(QString key);
    
    int size() const;
    bool isEmpty() const;
    unsigned int probeHash(unsigned int h0, unsigned int i) const;
    
    HashItem* items;
    int table_size;
    int item_count;
private:
};

#endif 