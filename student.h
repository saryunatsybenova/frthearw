#ifndef STUDENT_H
#define STUDENT_H

#include <QDate>
#include <QString>
#include "constants.h"

class Student {
public:
    QString name;
    int klass;
    QDate birth;
    Student() : name(""), birth(), klass(0) {}
    Student(const QString& g, int f, const QDate& d) : name(g), klass(f), birth(d) {}
};

struct StudentStorage {
    Student student[MAX_STUDENT];
    int studentCount = 0;
};

#endif 