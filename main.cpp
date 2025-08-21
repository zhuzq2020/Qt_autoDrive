#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Qt6 自动驾驶模拟器");
    w.resize(1000, 700);
    w.show();
    return a.exec();
}