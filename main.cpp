#include "mainwindow.h"
#include "page_login.h"
#include <QApplication>
#include "usersql.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    userSql sql;
    MainWindow w;


    return a.exec();
}
