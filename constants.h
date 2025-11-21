#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr int MIN_HASH_TABLE_SIZE = 1;
constexpr int MAX_HASH_TABLE_SIZE = 10000;
constexpr int MIN_CLASS = 1;
constexpr int MAX_CLASS = 11;
constexpr int MIN_GRADE = 2;
constexpr int MAX_GRADE = 5;
constexpr int MAX_GRADE_AGE_YEARS = 10;
constexpr int MAX_GRADE_AGE_DAYS = MAX_GRADE_AGE_YEARS * 365;
constexpr int MIN_STUDENT_AGE = 6;
constexpr int MAX_STUDENT_AGE = 18;
constexpr int SUBJECT_TYPES_COUNT = 6;
constexpr int MAX_STUDENT = 1000;
constexpr int MAX_GRADES = 10000;

static const char* SUBJECT_TYPES[] = {
    "Математика",
    "Русский язык",
    "Литература",
    "История",
    "Физика",
    "Информатика"
};

#endif 