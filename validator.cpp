#include "validator.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QFile>

const QStringList Validator::VALID_SUBJECT_TYPES = {
    "Математика",
    "Русский язык",
    "Литература",
    "История",
    "Физика",
    "Информатика"
};

const QRegularExpression Validator::DATE_REGEX = QRegularExpression(
    R"(^(\d{1,2})\.(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\.(\d{4})$)",
    QRegularExpression::CaseInsensitiveOption
);
const QHash<QString, int> Validator::MONTHS_MAP = {
    {"jan", 1}, {"feb", 2}, {"mar", 3}, {"apr", 4},
    {"may", 5}, {"jun", 6}, {"jul", 7}, {"aug", 8},
    {"sep", 9}, {"oct", 10}, {"nov", 11}, {"dec", 12}
};
const QRegularExpression Validator::STUDENT_NAME_REGEX = QRegularExpression("^[А-ЯЁ][а-яё]+$");

const int Validator::MAX_GRADE_ADVANCE_DAYS;

bool Validator::validateKlassNumber(int klass)
{
    return klass >= MIN_CLASS && klass <= MAX_CLASS;
}

bool Validator::validateStudentName(const QString& studentName)
{
    QStringList parts = studentName.trimmed().split(" ", Qt::SkipEmptyParts);
    if (parts.size() != 3) {
        return false;
    }
    
    for (const QString& part : parts) {
        if (!STUDENT_NAME_REGEX.match(part).hasMatch()) {
            return false;
        }
    }
    return true;
}

QDate Validator::parseDateFromString(const QString& date, bool* ok)
{
    if (ok) *ok = false;
    
    if (!isValidDateFormat(date)) {
        return QDate();
    }
    
    QStringList parts = date.split(".");
    if (parts.size() != 3) {
        return QDate();
    }
    
    // Преобразуем день и год
    bool dayOk, yearOk;
    int day = parts[0].toInt(&dayOk);
    int year = parts[2].toInt(&yearOk);
    
    if (!dayOk || !yearOk) {
        return QDate();
    }
    
    // Преобразуем название месяца в число
    QString monthName = parts[1].toLower();
    int month = MONTHS_MAP.value(monthName, -1);
    if (month == -1) {
        return QDate();
    }
    
    // Создаем объект QDate
    QDate qdate(year, month, day);
    if (ok) *ok = qdate.isValid();
    return qdate;
}

QString Validator::formatDateForDisplay(const QDate& date)
{
    return date.toString("dd.MM.yyyy");
}

// Дополнительный метод для форматирования в оригинальном стиле
QString Validator::formatDateInOriginalStyle(const QDate& date)
{
    static QStringList monthNames = {
        "jan", "feb", "mar", "apr", "may", "jun",
        "jul", "aug", "sep", "oct", "nov", "dec"
    };
    
    int month = date.month();
    if (month >= 1 && month <= 12) {
        return QString("%1.%2.%3")
            .arg(date.day())
            .arg(monthNames[month - 1])
            .arg(date.year());
    }
    return QString();
}

bool Validator::isValidDateFormat(const QString& date)
{
    return DATE_REGEX.match(date).hasMatch();
}

bool Validator::isValidDate(const QString& date)
{
    bool ok;
    QDate parsedDate = parseDateFromString(date, &ok);
    return ok && parsedDate.isValid();
}

bool Validator::isDateInFuture(const QString& date)
{
    bool ok;
    QDate checkInDate = parseDateFromString(date, &ok);
    if (!ok) return false;
    
    QDate currentDate = QDate::currentDate();
    return checkInDate >= currentDate;
}

bool Validator::isDateNotTooFar(const QString& date, int maxDays)
{
    bool ok;
    QDate checkInDate = parseDateFromString(date, &ok);
    if (!ok) return false;
    
    QDate currentDate = QDate::currentDate();
    QDate maxDate = currentDate.addDays(maxDays);
    
    return checkInDate <= maxDate;
}

bool Validator::isStudentAgeValid(const QDate& birthDate, QString& errorMessage)
{
    QDate currentDate = QDate::currentDate();
    
    // Проверяем что студент не слишком стар
    QDate minBirthDate = currentDate.addYears(-MAX_STUDENT_AGE);
    if (birthDate < minBirthDate) {
        errorMessage = formatErrorMessage("Дата рождения", 
            QString("студент не может быть старше %1 лет").arg(MAX_STUDENT_AGE));
        return false;
    }
    
    // Проверяем что студент не слишком молод
    QDate maxBirthDate = currentDate.addYears(-MIN_STUDENT_AGE);
    if (birthDate > maxBirthDate) {
        errorMessage = formatErrorMessage("Дата рождения", 
            QString("студент не может быть младше %1 лет").arg(MIN_STUDENT_AGE));
        return false;
    }
    
    return true;
}

bool Validator::validateStudent(const Student& student, QString& errorMessage)
{
    if (!validateKlassNumber(student.klass)) {
        errorMessage = formatErrorMessage("Класс", 
            QString("должен быть от %1 до %2").arg(MIN_CLASS).arg(MAX_CLASS));
        return false;
    }
    
    if (!validateStudentName(student.name)) {
        errorMessage = formatErrorMessage("ФИО студента", 
            "должно состоять из трёх слов (длина каждого слова >1), только русские буквы, первая буква каждого слова — заглавная");
        return false;
    }

    // Преобразуем QDate в строку для валидации
    QString birthDateStr = formatDateInOriginalStyle(student.birth);
    
    // Используем ранее написанные методы для проверки даты
    if (!isValidDate(birthDateStr)) {
        errorMessage = formatErrorMessage("Дата рождения", "некорректный формат или несуществующая дата");
        return false;
    }
    
    // Проверка что дата не в будущем
    if (isDateInFuture(birthDateStr)) {
        errorMessage = formatErrorMessage("Дата рождения", "не может быть в будущем");
        return false;
    }
    
    // Дополнительная проверка возраста студента
    if (!isStudentAgeValid(student.birth, errorMessage)) {
        return false;
    }
    
    return true;
}

bool Validator::validateSubjectType(const QString& subject)
{
    return VALID_SUBJECT_TYPES.contains(subject);
}

bool Validator::validateMark(int mark)
{
    return mark >= MIN_GRADE && mark <= MAX_GRADE;
}

bool Validator::validateGrade(const Grade& grade, QString& errorMessage)
{
    if (!validateMark(grade.mark)) {
        errorMessage = formatErrorMessage("Оценка", 
            QString("должна быть от %1 до %2").arg(MIN_GRADE).arg(MAX_GRADE));
        return false;
    }
    
    if (!validateStudentName(grade.FIO)) {
        errorMessage = formatErrorMessage("ФИО ученика", 
            "должно состоять из трёх слов (длина каждого слова >1), только русские буквы, первая буква каждого слова — заглавная");
        return false;
    }
    
    if (!validateSubjectType(grade.subject)) {
        errorMessage = formatErrorMessage("Предмет", "должен быть одним из: " + getSubjectTypeList());
        return false;
    }
    
    QString markDateStr = formatDateInOriginalStyle(grade.Mark_date);
    
    // Проверяем валидность даты
    if (!isValidDate(markDateStr)) {
        errorMessage = formatErrorMessage("Дата оценки", "некорректный формат или несуществующая дата");
        return false;
    }
    
    // Проверяем что дата оценки не в будущем
    if (isDateInFuture(markDateStr)) {
        errorMessage = formatErrorMessage("Дата оценки", "не может быть в будущем");
        return false;
    }
    
    // Проверяем что дата оценки не слишком старая (например, не старше 10 лет)
    if (!isDateNotTooFar(markDateStr, MAX_GRADE_AGE_DAYS)) {
        errorMessage = formatErrorMessage("Дата оценки", 
            QString("не может быть старше %1 лет").arg(MAX_GRADE_AGE_YEARS));
        return false;
    }
    
    return true;
}

bool Validator::validateFileExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists();
}

bool Validator::validateFileReadable(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }
    
    QFile file(filePath);
    return file.open(QIODevice::ReadOnly);
}

bool Validator::validateHashTableSize(int size)
{
    return size >= MIN_HASH_TABLE_SIZE && size <= MAX_HASH_TABLE_SIZE;
}

QString Validator::formatErrorMessage(const QString& field, const QString& error)
{
    return QString("Ошибка в поле '%1': %2").arg(field).arg(error);
}

QString Validator::getSubjectTypeList()
{
    return VALID_SUBJECT_TYPES.join(", ");
}
