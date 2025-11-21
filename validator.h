#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QString>
#include <QDate>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "student.h"
#include "grades.h"
#include "constants.h"

class Validator
{
public:
    static bool validateKlassNumber(int klass);
    static bool validateStudentName(const QString& studentName);
    
    static bool isValidDateFormat(const QString& date);
    static bool isValidDate(const QString& date);
    static bool isDateInFuture(const QString& date);
    static bool isDateNotTooFar(const QString& date, int maxDays = 365);
    static bool isStudentAgeValid(const QDate& birthDate, QString& errorMessage);
    
    static QDate parseDateFromString(const QString& date, bool* ok = nullptr);
    static QString formatDateForDisplay(const QDate& date);
    static QString formatDateInOriginalStyle(const QDate& date);

    static bool validateStudent(const Student& room, QString& errorMessage);

    static bool validateSubjectType(const QString& subject);
    static bool validateMark(int mark);
    static bool validateGrade(const Grade& grade, QString& errorMessage);

    static bool validateHashTableSize(int size);
    static bool validateFileExists(const QString& filePath);
    static bool validateFileReadable(const QString& filePath);

    static QString formatErrorMessage(const QString& field, const QString& error);
    static QString getSubjectTypeList();

private:
    static const QStringList VALID_SUBJECT_TYPES;
    static const QRegularExpression DATE_REGEX;
    static const QHash<QString, int> MONTHS_MAP;
    static const QRegularExpression STUDENT_NAME_REGEX;
    
    static const int MAX_GRADE_ADVANCE_DAYS = 365;
};

#endif 