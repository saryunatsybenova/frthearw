#ifndef GRADES_H
#define GRADES_H
#include <string>
#include <QDate>
#include <QString>
#include "constants.h"

class Grade {
public:
    QString FIO;
    QString subject;
    int mark;
    QDate Mark_date;

    Grade() : FIO(""), subject(""), mark(0), Mark_date() {}

    Grade(const QString& fio, const QString& sub, int m, const QDate& mdate)
        : FIO(fio), subject(sub), mark(m), Mark_date(mdate) {}

    bool operator<(const Grade& other) const {
        if (FIO != other.FIO)
            return FIO < other.FIO;
        return Mark_date < other.Mark_date;
    }
    bool operator==(const Grade& other) const {
        return FIO == other.FIO && subject == other.subject && mark == other.mark && Mark_date == other.Mark_date;
    }
    bool operator>(const Grade& other) const {
        return other < *this;
    }
};

struct GradeStorage {
    Grade grades[MAX_GRADES];
    int GradesCount = 0;
};

#endif 
