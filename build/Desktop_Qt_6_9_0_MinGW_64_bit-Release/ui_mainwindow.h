/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGraphicsView *graphicsView;
    QPushButton *btnLeft;
    QPushButton *btnRight;
    QPushButton *btnAccel;
    QPushButton *btnDecel;
    QPushButton *btnBrake;
    QLabel *statusLabel;
    QPushButton *btnFigure8;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(982, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName("graphicsView");
        graphicsView->setGeometry(QRect(190, 80, 531, 421));
        btnLeft = new QPushButton(centralwidget);
        btnLeft->setObjectName("btnLeft");
        btnLeft->setGeometry(QRect(80, 150, 93, 28));
        btnRight = new QPushButton(centralwidget);
        btnRight->setObjectName("btnRight");
        btnRight->setGeometry(QRect(80, 190, 93, 28));
        btnAccel = new QPushButton(centralwidget);
        btnAccel->setObjectName("btnAccel");
        btnAccel->setGeometry(QRect(80, 230, 93, 28));
        btnDecel = new QPushButton(centralwidget);
        btnDecel->setObjectName("btnDecel");
        btnDecel->setGeometry(QRect(80, 270, 93, 28));
        btnBrake = new QPushButton(centralwidget);
        btnBrake->setObjectName("btnBrake");
        btnBrake->setGeometry(QRect(80, 310, 93, 28));
        statusLabel = new QLabel(centralwidget);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setGeometry(QRect(760, 140, 201, 201));
        btnFigure8 = new QPushButton(centralwidget);
        btnFigure8->setObjectName("btnFigure8");
        btnFigure8->setGeometry(QRect(80, 80, 93, 28));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 982, 25));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        btnLeft->setText(QCoreApplication::translate("MainWindow", "\345\267\246\350\275\254", nullptr));
        btnRight->setText(QCoreApplication::translate("MainWindow", "\345\217\263\350\275\254", nullptr));
        btnAccel->setText(QCoreApplication::translate("MainWindow", "\345\212\240\351\200\237", nullptr));
        btnDecel->setText(QCoreApplication::translate("MainWindow", "\345\207\217\351\200\237", nullptr));
        btnBrake->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "\347\212\266\346\200\201", nullptr));
        btnFigure8->setText(QCoreApplication::translate("MainWindow", "8\345\255\227\345\236\213\350\267\257\347\272\277", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
