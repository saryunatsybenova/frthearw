#include "menu.h"
#include "./ui_menu.h"
#include <QTextStream>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "avltree.h"
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include "validator.h"
#include <QDate>
#include "grades.h"
#include "studenthashtable.h"
#include "list.h"
#include "gradesutils.h"
#include "studentutils.h"
#include "constants.h"
#include "validator.h"

void addIntToArray(int*& arr, int& count, int& capacity, int value) {
    if (count == capacity) {
        int newCapacity = (capacity == 0) ? 8 : capacity * 2;
        int* newArr = new int[newCapacity];
        for (int i = 0; i < count; ++i)
            newArr[i] = arr[i];
        delete[] arr;
        arr = newArr;
        capacity = newCapacity;
    }
    arr[count++] = value;
}

Menu::Menu(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Menu)
{
    ui->setupUi(this);
    studentHashTable = nullptr;
    reportGrade = nullptr;
    reportGradeCount = 0;
    GradeCreated = false;
}

Menu::~Menu()
{
    delete studentHashTable;
    delete ui;
}

void Menu::on_action_8_triggered()  { addStudent(); }
void Menu::on_action_10_triggered() { saveStudentToFile(); }
void Menu::on_action_16_triggered() { removeStudent(); }
void Menu::on_action_22_triggered() { findStudent(); }
void Menu::on_action_28_triggered() { saveGradeToFile(); }
void Menu::on_action_29_triggered() { addGrade(); }
void Menu::on_action_33_triggered() { removeGrade(); }
void Menu::on_action_35_triggered() { findGrade(); }

void Menu::showStudentInTable() {
    if (!studentHashTable) return;
    int n = studentHashTable->size();
    ui->tableWidget->setRowCount(n);
    ui->tableWidget->setColumnCount(4); 
    QStringList headers;
    headers << "Индекс" << "Ключ (ФИО)" << "Значение (Класс, Дата рождения)" << "Статус";
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    for (int i = 0; i < n; ++i) {
        const HashItem& item = studentHashTable->items[i];
        if (item.status == SlotStatus::OCCUPIED && item.index != -1) {
            const Student& r = studentStorage.student[item.index];
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(r.name));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(Validator::formatDateInOriginalStyle(r.birth) + ", " + QString::number(r.klass)));
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(static_cast<unsigned>(item.status))));
        } else {
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem("---"));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem("---"));
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(static_cast<unsigned>(item.status))));
        }
    }
}

void Menu::loadStudentFromFile()
{
    bool ok = false;
    int size = QInputDialog::getInt(this, "Размер хеш-таблицы", "Введите размер хеш-таблицы:", 25, MIN_HASH_TABLE_SIZE, MAX_HASH_TABLE_SIZE, 1, &ok);
    if (!ok) return;

    if (!Validator::validateHashTableSize(size)) {
        QMessageBox::warning(this, "Ошибка валидации", "Размер хеш-таблицы должен быть от 1 до 10000");
        return;
    }
}

void Menu::addStudent()
{
    if (!studentHashTable) {
        bool ok = false;
        int size = QInputDialog::getInt(this, "Размер хеш-таблицы", "Введите размер хеш-таблицы:", 25, MIN_HASH_TABLE_SIZE, MAX_HASH_TABLE_SIZE, 1, &ok);
        if (!ok) return;
        if (!Validator::validateHashTableSize(size)) {
            QMessageBox::warning(this, "Ошибка валидации", "Размер хеш-таблицы должен быть от 1 до 10000");
            return;
        }
        studentHashTable = new StudentHashTable(size);
    }
    bool ok;
    QString name = QInputDialog::getText(this, "Добавить ученика", "Введите ФИО ученика:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (!Validator::validateStudentName(name)) {
        QMessageBox::warning(this, "Ошибка валидации", "ФИО ученика должно состоять из трёх слов (длина каждого слова >1), только русские буквы, первая буква каждого слова — заглавная");
        return;
    }
    
    int klass = QInputDialog::getInt(this, "Добавить ученика", "Введите класс:", MIN_CLASS, MIN_CLASS, MAX_CLASS, 1, &ok);
    if (!ok) return;
    if (!Validator::validateKlassNumber(klass)) {
        QMessageBox::warning(this, "Ошибка валидации", "Класс должен быть от 1 до 11");
        return;
    }
    QString DateStr = QInputDialog::getText(this, "Дата рождения", "Введите дату (DD.MMM.YYYY):");
    if (DateStr.isEmpty()) return;
    bool dateOk;
    QDate InDate = Validator::parseDateFromString(DateStr, &dateOk);
    if (!dateOk || !Validator::isValidDate(Validator::formatDateForDisplay(InDate))) {
        QMessageBox::warning(this, "Ошибка валидации", "Дата рождения должна быть в формате DD.MMM.YYYY");
        return;
    }
    Student r(name, klass, InDate);
    QString errorMessage;
    if (!Validator::validateStudent(r, errorMessage)) {
        QMessageBox::warning(this, "Ошибка валидации", errorMessage);
        return;
    }
    QString errorReason;
    bool inserted = StudentUtils::insertStudent(r, *studentHashTable, studentStorage, &errorReason);
    if (inserted) {
        QString debug = QString("запись %1\n").arg(name);
        QStringList lines = StudentUtils::getHashChainDebug(studentHashTable, name, studentStorage);
        for (const QString& line : lines) debug += line + "\n";
        ui->textEdit->setPlainText(debug);
        QMessageBox::information(this, "Добавление", "Ученик успешно добавлен!");
        showStudentInTable();
    } else {
        if (errorReason == "duplicate") {
            QMessageBox::warning(this, "Ошибка", "Такой ученик уже существует!");
        } else if (errorReason == "full") {
            QMessageBox::warning(this, "Ошибка", "Таблица заполнена!");
        } else if (errorReason == "storage_full") {
            QMessageBox::warning(this, "Ошибка", "Достигнут лимит оценок!");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить ученика!");
        }
    }
}

void Menu::removeStudent()
{
    if (!studentHashTable) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник учеников (хеш-таблицу)!");
        return;
    }
    if (studentHashTable->isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Справочник учеников пуст!");
        return;
    }
    bool ok;
    QString name = QInputDialog::getText(this, "Удалить ученика", "Введите ФИО ученика:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    QString debug = QString("Удаление %1\n").arg(name);
    QStringList lines = StudentUtils::getHashChainDebug(studentHashTable, name, studentStorage);
    for (const QString& line : lines) debug += line + "\n";
    debug += "\n";
    if (StudentUtils::removeStudent(name, *studentHashTable, studentStorage)) {
        ui->textEdit->setPlainText(debug);
        int removedBookings = GradeUtils::removeAllGradesByFIO(gradetree, name, gradeStorage);
        QMessageBox::information(this, "Удаление", QString("ученик успешно удалён!\nУдалено связанных оценок: %1").arg(removedBookings));
        showStudentInTable();
        showGradeInTableInOrder();
    } else {
        ui->textEdit->setPlainText(debug);
        QMessageBox::warning(this, "Ошибка", "Такого ученика нет!");
    }
}

void Menu::findStudent()
{
    if (!studentHashTable) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник учеников (хеш-таблицу)!");
        return;
    }
    if (studentHashTable->isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Справочник учеников пуст!");
        return;
    }
    bool ok;
    QString name = QInputDialog::getText(this, "Найти ученика", "Введите ФИО ученика:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    auto res = StudentUtils::findStudent(name, *studentHashTable, studentStorage);
    QString debug = QString("Поиск %1\n").arg(name);
    QStringList lines = StudentUtils::getHashChainDebug(studentHashTable, name, studentStorage);
    for (const QString& line : lines) debug += line + "\n";
    ui->textEdit->setPlainText(debug);
    if (res.first.name != "") {
        QMessageBox::information(this, "Найдено", QString("ФИО: %1\nКласс: %2\nДата рождения: %3\n(Шагов поиска: %4)")
            .arg(res.first.name).arg(res.first.klass).arg(Validator::formatDateInOriginalStyle(res.first.birth)).arg(res.second));
    } else {
        QMessageBox::warning(this, "Не найдено", "Такой ученик не найден!");
    }
}

void Menu::saveStudentToFile()
{
    if (studentStorage.studentCount == 0) {
        QMessageBox::information(this, "Сохранение", "Нет данных для сохранения.");
        return;
    }
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить учеников", "", "Text Files (*.txt)");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка файла", QString("Не удалось открыть файл %1").arg(filename));
        return;
    }
    QTextStream out(&file);
    for (int i = 0; i < studentStorage.studentCount; ++i) {
        const Student& r = studentStorage.student[i];
            out << r.name << " " << r.klass << " " << Validator::formatDateInOriginalStyle(r.birth) << "\n";
    }
    file.close();
    if (studentStorage.studentCount == 0) {
        QMessageBox::information(this, "Сохранение", "Нет данных для сохранения.");
    } else {
        QMessageBox::information(this, "Сохранение", QString("Справочник учеников успешно сохранен. Сохранено %1 записей.").arg(studentStorage.studentCount));
    }
}

void Menu::showGradeInTableInOrder() {
    int count = 0;
    int indices[MAX_GRADES];
    GradeUtils::inOrderGrades(gradetree, indices, count, gradeStorage);
    ui->tableWidget_2->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        int idx = indices[i];
        if (idx >= 0 && idx < gradeStorage.GradesCount) {
            const Grade& b = gradeStorage.grades[idx];
            ui->tableWidget_2->setItem(i, 0, new QTableWidgetItem(b.FIO));
            ui->tableWidget_2->setItem(i, 1, new QTableWidgetItem(b.subject));
            ui->tableWidget_2->setItem(i, 2, new QTableWidgetItem(QString::number(b.mark)));
            ui->tableWidget_2->setItem(i, 3, new QTableWidgetItem(Validator::formatDateInOriginalStyle(b.Mark_date)));
        }
    }
}

void Menu::addGrade() {
    if (!checkStudentHashTableExists()) {
        return;
    }
    bool ok;
    QString name = QInputDialog::getText(this, "Найти ученика", "Введите ФИО ученика:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (!Validator::validateStudentName(name)) {
        QMessageBox::warning(this, "Ошибка валидации", "ФИО ученика должно состоять из трёх слов (длина каждого слова >1), только русские буквы, первая буква каждого слова — заглавная");
        return;
    }
    auto res = StudentUtils::findStudent(name, *studentHashTable, studentStorage);
    if (res.first.name == "") {
        QMessageBox::warning(this, "Ошибка", "Такого ученика нет в справочнике. Сначала добавьте ученика.");
        return;
    }
    Grade* marks = nullptr;
    int markCount = 0;
    auto node = gradetree.find(name);
    if (node && node->value) {
        markCount = node->value->count();
        if (markCount > 0) {
            marks = new Grade[markCount];
            int arrIdx = 0;
            node->value->forEach([&](int gradeIdx) {
                marks[arrIdx++] = gradeStorage.grades[gradeIdx];
            });
        }
    }
    QStringList typeList;
    for (int i = 0; i < SUBJECT_TYPES_COUNT; ++i) typeList << SUBJECT_TYPES[i];
    QString type = QInputDialog::getItem(this, "Добавить оценку", "Предмет:", typeList, 0, false, &ok);
    if (!ok || type.isEmpty()) return;
    if (!Validator::validateSubjectType(type)) {
        QMessageBox::warning(this, "Ошибка валидации", "Предмет должен быть одним из: " + Validator::getSubjectTypeList());
        return;
    }
    int mark = QInputDialog::getInt(this, "Оценка", "Введите оценку:", MIN_GRADE, MIN_GRADE, MAX_GRADE, 1, &ok);
    if (!ok) return;
    if (!Validator::validateMark(mark)) {
        QMessageBox::warning(this, "Ошибка валидации", "Оценка должна быть от 2 до 5");
        return;
    }
    QString DateStr = QInputDialog::getText(this, "Дата выставления", "Введите дату (DD.MMM.YYYY):");
    if (DateStr.isEmpty()) return;
    bool dateOk;
    QDate InDate = Validator::parseDateFromString(DateStr, &dateOk);
    if (!dateOk || !Validator::isValidDate(Validator::formatDateForDisplay(InDate))) {
        QMessageBox::warning(this, "Ошибка валидации", "Дата выставления должна быть в формате DD.MMM.YYYY");
        return;
    }
    Grade b;
    b.FIO = name;
    b.subject = type;
    b.mark = mark;
    b.Mark_date = InDate;
    QString errorMessage;
    if (!Validator::validateGrade(b, errorMessage)) {
        QMessageBox::warning(this, "Ошибка валидации", errorMessage);
        delete[] marks;
        return;
    }
    int idx = -1;
    if (gradeStorage.GradesCount < MAX_GRADES) {
        idx = gradeStorage.GradesCount++;
        gradeStorage.grades[idx] = b;
    } else {
            QMessageBox::warning(this, "Ошибка", "Достигнут лимит оценок!");
            delete[] marks;
            return;
    }
    GradeUtils::insertGrade(gradetree, idx, gradeStorage);
    GradeCreated = true;
    QMessageBox::information(this, "Добавление", "Оценка успешно добавлена!");
    showGradeInTableInOrder();
    delete[] marks;
}

void Menu::removeGrade() {
    if (!checkStudentHashTableExists()) {
        return;
    }
    if (!checkGradeCreated()) {
        return;
    }
    if (!checkGradeNotEmpty()) {
        return;
    }
    bool ok;
    QString name = QInputDialog::getText(this, "Удалить", "Введите ФИО ученика:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (!Validator::validateStudentName(name)) {
        QMessageBox::warning(this, "Ошибка валидации", "ФИО ученика должно состоять из трёх слов (длина каждого слова >1), только русские буквы, первая буква каждого слова — заглавная");
        return;
    }
    QString dateStr = QInputDialog::getText(this, "Удалить", "Дата выставления (DD.MMM.YYYY):");
    if (dateStr.isEmpty()) return;
    bool dateOk;
    QDate date = Validator::parseDateFromString(dateStr, &dateOk);
    if (!dateOk || !Validator::isValidDateFormat(dateStr)) {
        QMessageBox::warning(this, "Ошибка валидации", "Дата должна быть в формате DD.MMM.YYYY");
        return;
    }
    auto node = gradetree.find(name);
    if (!node || !node->value) {
        QMessageBox::warning(this, "Ошибка", "Оценка с указанным ФИО и датой выставления не найдена!");
        return;
    }
    auto found = GradeUtils::findGradeByDate(node->value, date, gradeStorage);
    if (!found) {
        QMessageBox::warning(this, "Ошибка", "Оценка с указанным ФИО и датой выставления не найдена!");
        return;
    }
    int removeIdx = found->gradeIndex;
    bool removed = GradeUtils::removeGradeByDate(node->value, date, gradeStorage);
    if (node->value->isEmpty()) {
        delete node->value;
        gradetree.remove(name);
    }
    if (removed) {
        int lastIdx = gradeStorage.GradesCount - 1;
        if (removeIdx != lastIdx) {
            gradeStorage.grades[removeIdx] = gradeStorage.grades[lastIdx];
            gradetree.inOrder([&](const QString& key, List* list) {
                if (list) {
                    list->forEach([&](int& idx) {
                        if (idx == lastIdx) idx = removeIdx;
                    });
                }
            });
        }
        gradeStorage.GradesCount--;
        QMessageBox::information(this, "Удаление", "Оценка успешно удалена!");
        showGradeInTableInOrder();
    } else {
        QMessageBox::warning(this, "Ошибка", "Оценка с указанным ФИО и датой выставления не найдена!");
    }
}

void Menu::findGrade() {
    if (!checkStudentHashTableExists()) {
        return;
    }
    if (!checkGradeCreated()) {
        return;
    }
    if (!checkGradeNotEmpty()) {
        return;
    }
    bool ok;
    QString name = QInputDialog::getText(this, "Поиск", "Введите ФИО ученика:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (!Validator::validateStudentName(name)) {
        QMessageBox::warning(this, "Ошибка валидации", "ФИО ученика должно состоять из трёх слов (длина каждого слова >1), только русские буквы, первая буква каждого слова — заглавная");
        return;
    }
    QString dateStr = QInputDialog::getText(this, "Поиск", "Дата выставления (DD.MMM.YYYY):");
    if (dateStr.isEmpty()) return;
    bool dateOk;
    QDate date = Validator::parseDateFromString(dateStr, &dateOk);
    if (!dateOk || !Validator::isValidDateFormat(dateStr)) {
        QMessageBox::warning(this, "Ошибка валидации", "Дата должна быть в формате DD.MMM.YYYY");
        return;
    }
    int steps = 0;
    int foundIndex = -1;
    auto node = gradetree.find(name);
    if (node && node->value) {
        auto found = GradeUtils::findGradeByDate(node->value, date, gradeStorage, steps);
        if (found) foundIndex = found->gradeIndex;
    }

    steps = 0;
    auto avlNode = gradetree.getRoot();
    while (avlNode) {
        steps++;
        if (name < avlNode->key) {
            avlNode = avlNode->left;
        } else if (name > avlNode->key) {
            avlNode = avlNode->right;
        } else {
            if (avlNode->value) {
                auto found = GradeUtils::findGradeByDate(avlNode->value, date, gradeStorage, steps);
                if (found) foundIndex = found->gradeIndex;
            }
            break;
        }
    }
    if (foundIndex != -1) {
        const Grade& b = gradeStorage.grades[foundIndex];
        QString info = QString("ФИО: %1\nПредмет: %2\nОценка: %3\nДата выставления: %4\nШагов поиска (сравнений): %5")
            .arg(b.FIO)
            .arg(b.subject)
            .arg(b.mark)
            .arg(b.Mark_date.toString("dd.MMM.yyyy"))
            .arg(steps);
        QMessageBox::information(this, "Найдено", info);
    } else {
        QMessageBox::warning(this, "Не найдено", "Оценка с указанным ФИО и датой выставления не найдена.");
    }
}

void Menu::saveGradeToFile() {
    int savedCount = 0;
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить оценки", "", "Text Files (*.txt)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка файла", QString("Не удалось открыть файл %1").arg(filename));
        return;
    }
    QTextStream out(&file);
    for (int i = 0; i < gradeStorage.GradesCount; ++i) {
        const Grade& b = gradeStorage.grades[i];
            out << b.FIO << " " << b.subject << " "
                << b.mark << " " << b.Mark_date.toString("dd.MMM.yyyy") << "\n";
    }
    file.close();
    if (gradeStorage.GradesCount == 0) {
        QMessageBox::information(this, "Сохранение", "Нет данных для сохранения.");
    } else {
        QMessageBox::information(this, "Сохранение", QString("Сохранено %1 оценок").arg(gradeStorage.GradesCount));
    }
}

void Menu::on_action_21_triggered() { //формирование отчета
    if (!studentHashTable || !GradeCreated) {
        QMessageBox::warning(this, "Данные не загружены",
            "Для формирования отчёта необходимо сначала загрузить справочники учеников и оценок.");
        return;
    }

    AVLTree<QString, List*> filterTree;
    int allGradeCount = 0;
    GradeUtils::inOrderGrades(gradetree, nullptr, allGradeCount, gradeStorage);
    if (allGradeCount > 0) {
        int* indices = new int[allGradeCount];
        GradeUtils::inOrderGrades(gradetree, indices, allGradeCount, gradeStorage);
        for (int i = 0; i < allGradeCount; ++i) {
            const Grade& b = gradeStorage.grades[indices[i]];
            const QString& date = Validator::formatDateInOriginalStyle(b.Mark_date);
            auto node = filterTree.find(date);
            if (node && node->value) {
                node->value->add(indices[i]);
            } else {
                auto* list = new List();
                list->add(indices[i]);
                filterTree.insert(date, list);
            }
        }
        delete[] indices;
    }

    QStringList typeList;
    for (int i = 0; i < SUBJECT_TYPES_COUNT; ++i) typeList << SUBJECT_TYPES[i];
    bool okType = false;
    QString selectedType = QInputDialog::getItem(this, "Фильтр по типу", "Предмет:", typeList, 0, false, &okType);
    if (!okType || selectedType.isEmpty()) return;

    bool okBirthday = false;
    QString BirthDate = QInputDialog::getText(this, "Фильтр по дате рождения", "Дата рождения (DD.MMM.YYYY):", QLineEdit::Normal, "", &okBirthday);
    if (!okBirthday || BirthDate.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Дата рождения обязательна для формирования отчета");
        return;
    }

    if (!Validator::isValidDateFormat(BirthDate)) {
        QMessageBox::warning(this, "Ошибка валидации", "Дата рождения должна быть в формате DD.MMM.YYYY");
        return;
    }

    bool okDate = false;
    QString filterDate = QInputDialog::getText(this, "Фильтр по дате", "Дата выставления (DD.MMM.YYYY):", QLineEdit::Normal, "", &okDate);
    if (!okDate || filterDate.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Дата выставления обязательна для формирования отчета");
        return;
    }

    if (!Validator::isValidDateFormat(filterDate)) {
        QMessageBox::warning(this, "Ошибка валидации", "Дата выставления должна быть в формате DD.MMM.YYYY");
        return;
    }

    Grade* filteredGrade = nullptr;
    int filteredGradeCount = 0;
    int filteredGradeCapacity = 0;
    GradeUtils::filterByBirthdayAndSubject(filterTree, studentHashTable, filteredGrade, filteredGradeCount, filteredGradeCapacity, filterDate, QDate::fromString(BirthDate, "dd.MMM.yyyy"), selectedType, gradeStorage, studentStorage);
    
    QString report = "ФИО | Класс | Дата рождения | Предмет | Оценка| Дата выставления\n";
    report += "------------------------------------------------\n";
    
    for (int i = 0; i < filteredGradeCount; ++i) {
        const Grade& b = filteredGrade[i];
        auto res = StudentUtils::findStudent(b.FIO, *studentHashTable, studentStorage);
        if (res.first.name != "") {
            report += QString("%1 | %2 | %3 | %4 | %5 | %6\n")
                .arg(res.first.name)
                .arg(res.first.klass)
                .arg(Validator::formatDateInOriginalStyle(res.first.birth))
                .arg(b.subject)
                .arg(b.mark)
                .arg(b.Mark_date.toString("dd.MMM.yyyy"));
        }
    }
    
    if (reportGrade) delete[] reportGrade;
    reportGrade = nullptr;
    reportGradeCount = 0;
    int reportGradeCapacity = 0;
    for (int i = 0; i < filteredGradeCount; ++i) {
        GradeUtils::addGradeToArray(reportGrade, reportGradeCount, reportGradeCapacity, filteredGrade[i]);
    }
    
    if (filteredGrade) { delete[] filteredGrade; filteredGrade = nullptr; }
    
    if (reportGradeCount == 0) {
        QMessageBox::warning(this, "Предупреждение", "Не найдено оценок с заданными полями");
    } else {
        ui->textEdit->setPlainText(report);
        QMessageBox::information(this, "Отчет успешно сформирован", "Отчет успешно сформирован");
    }
}

void Menu::on_action_25_triggered() { //печать дерева
    if (!GradeCreated) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник оценок (AVL-дерево)!");
        return;
    }
    QString debugStr;
    GradeUtils::printGradesTree(gradetree, debugStr, gradeStorage,
        [](const Grade& b) {
            return b.subject.toStdString() + " " + std::to_string(b.mark) + " " + Validator::formatDateInOriginalStyle(b.Mark_date).toStdString();
        });
    ui->textEdit->setPlainText(debugStr);
}

void Menu::on_action_23_triggered() {  //отладка
    if (!studentHashTable) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник учеников (хеш-таблицу)!");
        return;
    }
    QString debug;
    for (int i = 0; i < studentHashTable->size(); ++i) {
        if (studentHashTable->items[i].status == SlotStatus::OCCUPIED) {
            QString name = studentStorage.student[studentHashTable->items[i].index].name;
            debug += QString("запись %1\n").arg(name);
            QStringList lines = StudentUtils::getHashChainDebug(studentHashTable, name, studentStorage);
            for (const QString& line : lines) debug += line + "\n";
            debug += "\n";
        }
    }
    ui->textEdit->setPlainText(debug);
}

void Menu::on_action_12_triggered() { //показать отчет
    if (!reportGrade || reportGradeCount == 0) {
        QMessageBox::warning(this, "Предупреждение", "Сначала создайте отчет");
        return;
    }
    QString report;
    for (int i = 0; i < reportGradeCount; ++i) {
        const Grade& b = reportGrade[i];
        auto res = StudentUtils::findStudent(b.FIO, *studentHashTable, studentStorage);
        report += QString("ФИО: %1, Класс: %2, Дата рождения: %3, Предмет: %4, Оценка: %5, Дата выставления: %6\n")
            .arg(res.first.name)
            .arg(res.first.klass)
            .arg(Validator::formatDateInOriginalStyle(res.first.birth))
            .arg(b.subject)
            .arg(b.mark)
            .arg(b.Mark_date.toString("dd.MMM.yyyy"));
    }
    
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Отчет");
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QTextEdit* textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(report);
    layout->addWidget(textEdit);
    dialog->setLayout(layout);
    dialog->resize(700, 500);
    dialog->exec();
}

void Menu::on_action_26_triggered() { //сохранить отчет
    if (!reportGrade || reportGradeCount == 0) {
        QMessageBox::warning(this, "Предупреждение", "Сначала создайте отчет");
        return;
    }
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить отчет", "", "Text Files (*.txt)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    
    for (int i = 0; i < reportGradeCount; ++i) {
        const Grade& b = reportGrade[i];
        auto res = StudentUtils::findStudent(b.FIO, *studentHashTable, studentStorage);
        out << res.first.name << " "
            << res.first.klass << " "
            << Validator::formatDateInOriginalStyle(res.first.birth) << " "
            << b.subject << " "
            << b.mark << " "
            << b.Mark_date.toString("dd.MMM.yyyy") << "\n";
    }
    file.close();
}

void Menu::on_action_20_triggered() {
    studentStorage.studentCount = 0;
    gradeStorage.GradesCount = 0;
    bool ok = false;
    int size = QInputDialog::getInt(this, "Размер хеш-таблицы", "Введите размер хеш-таблицы:", 25, MIN_HASH_TABLE_SIZE, MAX_HASH_TABLE_SIZE, 1, &ok);
    if (!ok) return;
    if (!Validator::validateHashTableSize(size)) {
        QMessageBox::warning(this, "Ошибка валидации", "Размер хеш-таблицы должен быть от 1 до 10000");
        return;
    }
    if (studentHashTable) delete studentHashTable;
    studentHashTable = new StudentHashTable(size);
    gradetree = AVLTree<QString, List*>();
    GradeCreated = false;
    showGradeInTableInOrder();
    QString filename = QFileDialog::getOpenFileName(this, "Выберите файл с учениками", "", "Text Files (*.txt);;All Files (*)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка файла", QString("Не удалось открыть файл %1").arg(filename));
        return;
    }
    int validCount = 0, invalidCount = 0, duplicates = 0, overflow = 0;
    int validTotal = 0;
    QStringList errorLines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList parts = line.split(" ", Qt::SkipEmptyParts);
        if (parts.size() < 3) {
            errorLines << QString("%1 — Ошибка: недостаточно полей").arg(line);
            invalidCount++;
            continue;
        }
        int klass = 0;
        QString fio = ""; 
        QDate birthday;
        bool okKlass = false;

        for (int i = 1; i < parts.size() - 2; ++i) {
            if (i > 1) fio += " ";
            fio += parts[i];
        }

        if (parts.size() >= 3) klass = parts[3].toInt(&okKlass);
        if (parts.size() >= 4) birthday = QDate::fromString(parts[4], "dd.MMM.yyyy");

        Student r(fio, klass, birthday);
        QString errorMessage;
        if (Validator::validateStudent(r, errorMessage)) {
            validTotal++; 
            QString errorReason;
            bool inserted = StudentUtils::insertStudent(r, *studentHashTable, studentStorage, &errorReason);
            if (inserted) {
                validCount++;
            } else {
                if (errorReason == "duplicate") {
                    duplicates++;
                } else {
                    overflow++;
                }
            }
        } else {
            errorLines << QString("%1 — %2").arg(line).arg(errorMessage);
            invalidCount++;
        }
    }
    file.close();
    ui->textEdit->setPlainText(errorLines.join("\n"));
    QString message = QString("Найдено корректных записей: %1\nУспешно загружено в таблицу: %2\nПропущено из-за дубликатов: %3\nПропущено из-за переполнения: %4")
                      .arg(validTotal)
                      .arg(validCount)
                      .arg(duplicates)
                      .arg(overflow);
    if (invalidCount > 0) {
        message += QString("\nПропущено %1 некорректных записей.").arg(invalidCount);
    }
    QMessageBox::information(this, "Результаты загрузки", message);
    showStudentInTable();
}

void Menu::on_action_31_triggered() {
    if (!studentHashTable) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник учеников (хеш-таблицу)!");
        return;
    }
    gradeStorage.GradesCount = 0;
    QString filename = QFileDialog::getOpenFileName(this, "Выберите файл с оценками", "", "Text Files (*.txt);;All Files (*)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка файла", QString("Не удалось открыть файл %1").arg(filename));
        return;
    }
    GradeCreated = true;
    gradetree = AVLTree<QString, List*>();
    int validCount = 0, invalidCount = 0, overflow = 0;
    QStringList errorLines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList parts = line.split(" ", Qt::SkipEmptyParts);
        if (parts.size() < 4) {
            errorLines << QString("%1 — Ошибка: недостаточно полей").arg(line);
            invalidCount++;
            continue;
        }

        QString fio = "";
        for (int i = 1; i < parts.size() - 3; ++i) {
            if (i > 1) fio += " ";
            fio += parts[i];
        }
        QString subject = parts[3];
        int mark = parts[4].toInt();
        QDate markDate = QDate::fromString(parts[5], "dd.MMM.yyyy");
        Grade b(fio, subject, mark, markDate);
        QString errorMessage;
        if (!Validator::validateGrade(b, errorMessage)) {
            errorLines << QString("%1 — %2").arg(line).arg(errorMessage);
            invalidCount++;
            continue;
        }
        auto roomRes = StudentUtils::findStudent(fio, *studentHashTable, studentStorage);
        if (roomRes.first.name == "") {
            errorLines << QString("%1 — Ученик %2 отсутствует в справочнике учеников").arg(line).arg(fio);
            invalidCount++;
            continue;
        }
        if (Validator::validateGrade(b, errorMessage)) {
            int marksCount = 0;
            Grade* marks = nullptr;
            auto node = gradetree.find(b.FIO);
            if (node && node->value) {
                marksCount = node->value->count();
                if (marksCount > 0) {
                    marks = new Grade[marksCount];
                    int arrIdx = 0;
                    node->value->forEach([&](int gradeIdx) {
                        marks[arrIdx++] = gradeStorage.grades[gradeIdx];
                    });
                }
            }
            delete[] marks;
            int idx = -1;
            if (gradeStorage.GradesCount < MAX_GRADES) {
                idx = gradeStorage.GradesCount++;
                gradeStorage.grades[idx] = b;
            } else {
                    overflow++;
                    continue;
            }
            GradeUtils::insertGrade(gradetree, idx, gradeStorage);
            validCount++;
        } else {
            errorLines << QString("%1 — %2: %3").arg(fio).arg(subject).arg(errorMessage);
            invalidCount++;
        }
    }
    file.close();
    ui->textEdit->setPlainText(errorLines.join("\n"));
    QString message = QString("Найдено корректных записей: %1\nУспешно загружено в дерево: %2\nПропущено %3 некорректных записей.")
                      .arg(validCount + invalidCount)
                      .arg(validCount)
                      .arg(invalidCount);
    QMessageBox::information(this, "Результаты загрузки", message);
    showGradeInTableInOrder();
}

void Menu::showStudentArrayInTable() { //показать справочник 1
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(3);
    QStringList headers;
    headers << "ФИО" << "Класс" << "Дата рождения";
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    int row = 0;
    for (int i = 0; i < studentStorage.studentCount; ++i) {
        const Student& r = studentStorage.student[i];
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(r.name));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(r.klass)));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(r.birth.toString("dd.MMM.yyyy")));
        row++;
    }
}

void Menu::showGradeArrayInTable() { //показать справочник 2
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->setColumnCount(4);
    QStringList headers;
    headers << "ФИО" << "Предмет" << "Оценка" << "Дата выставления";
    ui->tableWidget_2->setHorizontalHeaderLabels(headers);
    int row = 0;
    for (int i = 0; i < gradeStorage.GradesCount; ++i) {
        const Grade& b = gradeStorage.grades[i];
        ui->tableWidget_2->insertRow(row);
        ui->tableWidget_2->setItem(row, 0, new QTableWidgetItem(b.FIO));
        ui->tableWidget_2->setItem(row, 1, new QTableWidgetItem(b.subject));
        ui->tableWidget_2->setItem(row, 2, new QTableWidgetItem(QString::number(b.mark)));
        ui->tableWidget_2->setItem(row, 3, new QTableWidgetItem(b.Mark_date.toString("dd.MMM.yyyy")));
        row++;
    }
}

void Menu::on_action_34_triggered() {
    if (!studentHashTable) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник учеников (хеш-таблицу)!");
        return;
    }
    showStudentArrayInTable();
}

void Menu::on_action_4_triggered() {
    if (!studentHashTable) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник учеников (хеш-таблицу)!");
        return;
    }
    showStudentInTable();
}

void Menu::on_action_42_triggered() {
    if (!GradeCreated) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник оценок (AVL-дерево)!");
        return;
    }
    showGradeArrayInTable();
}

void Menu::on_action_39_triggered() {
    if (!GradeCreated) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник оценок (AVL-дерево)!");
        return;
    }
    showGradeInTableInOrder();
}

void Menu::on_action_37_triggered() {
    if (!studentHashTable || !GradeCreated) {
        QMessageBox::warning(this, "Данные не загружены",
            "Для печати дерева фильтрации необходимо сначала загрузить справочники учеников и оценок.");
        return;
    }
    AVLTree<QDate, List*, DateLess> filterTree;
    
    // Строим дерево фильтрации напрямую по массиву бронирований
    for (int i = 0; i < gradeStorage.GradesCount; ++i) {
        const Grade& b = gradeStorage.grades[i];
        const QDate& date = b.Mark_date;
        auto node = filterTree.find(date);
        if (node && node->value) {
            node->value->add(i);
        } else {
            auto* list = new List();
            list->add(i);
            filterTree.insert(date, list);
        }
    }
    
    QString debugStr;
    GradeUtils::printGradesTree(filterTree, debugStr, gradeStorage,
        [](const Grade& b) {
            return b.FIO + " " + b.subject + " " + QString::number(b.mark);
        });
    ui->textEdit->setPlainText(debugStr);
}

bool Menu::checkStudentHashTableExists() {
    if (!studentHashTable) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник учеников (хеш-таблицу)!");
        return false;
    }
    return true;
}

bool Menu::checkGradeCreated() {
    if (!GradeCreated) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите справочник оценок (AVL-дерево)!");
        return false;
    }
    return true;
}

bool Menu::checkGradeNotEmpty() {
    if (GradeUtils::isGradesEmpty(gradetree)) {
        QMessageBox::warning(this, "Ошибка", "Справочник оценок пуст!");
        return false;
    }
    return true;
}
