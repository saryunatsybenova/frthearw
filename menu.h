#ifndef MENU_H
#define MENU_H

#include <QMainWindow>
#include <QString>
#include "studenthashtable.h"
#include "avltree.h"
#include "grades.h"
#include "list.h"
#include "student.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Menu;
}
QT_END_NAMESPACE

class Menu : public QMainWindow
{
    Q_OBJECT

public:
    Menu(QWidget *parent = nullptr);
    ~Menu();

private:
    Ui::Menu *ui;
    StudentHashTable* studentHashTable;
    AVLTree<QString, List*> gradetree;
    Grade* reportGrade;
    int reportGradeCount;
    bool GradeCreated = false;
    StudentStorage studentStorage;
    GradeStorage gradeStorage;
    void showGradeInTableInOrder();
    void showStudentInTable();
    void showStudentArrayInTable();
    void showGradeArrayInTable();

private slots:
    void loadStudentFromFile();
    void addStudent();
    void removeStudent();
    void findStudent();
    void saveStudentToFile();
    void on_action_6_triggered();
    void on_action_8_triggered();
    void on_action_10_triggered();
    void on_action_16_triggered();
    void on_action_22_triggered();
    void on_action_24_triggered();
    void on_action_28_triggered();
    void on_action_29_triggered();
    void on_action_33_triggered();
    void addGrade();
    void removeGrade();
    void findGrade();
    void saveGradeToFile();
    void on_action_21_triggered(); 
    void on_action_12_triggered(); 
    void on_action_26_triggered(); 
    void on_action_25_triggered(); 
    void on_action_4_triggered(); 
    void on_action_23_triggered(); 
    void on_action_35_triggered();
    void on_action_20_triggered();
    void on_action_31_triggered();
    void on_action_34_triggered();
    void on_action_42_triggered();
    void on_action_39_triggered();
    void on_action_37_triggered();

private:
    bool checkStudentHashTableExists();
    bool checkGradeCreated();
    bool checkGradeNotEmpty();
};

#endif 
