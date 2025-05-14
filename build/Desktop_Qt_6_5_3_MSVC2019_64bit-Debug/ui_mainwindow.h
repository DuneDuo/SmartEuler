/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QPushButton *btnUpload;
    QPushButton *captureButton;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *vboxLayout1;
    QLabel *lblPreview;
    QLabel *resultLabel;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        vboxLayout = new QVBoxLayout(centralwidget);
        vboxLayout->setObjectName("vboxLayout");
        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName("hboxLayout");
        btnUpload = new QPushButton(centralwidget);
        btnUpload->setObjectName("btnUpload");

        hboxLayout->addWidget(btnUpload);

        captureButton = new QPushButton(centralwidget);
        captureButton->setObjectName("captureButton");

        hboxLayout->addWidget(captureButton);


        vboxLayout->addLayout(hboxLayout);

        scrollArea = new QScrollArea(centralwidget);
        scrollArea->setObjectName("scrollArea");
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignCenter);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        vboxLayout1 = new QVBoxLayout(scrollAreaWidgetContents);
        vboxLayout1->setObjectName("vboxLayout1");
        lblPreview = new QLabel(scrollAreaWidgetContents);
        lblPreview->setObjectName("lblPreview");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lblPreview->sizePolicy().hasHeightForWidth());
        lblPreview->setSizePolicy(sizePolicy);
        lblPreview->setScaledContents(false);
        lblPreview->setAlignment(Qt::AlignCenter);
        lblPreview->setStyleSheet(QString::fromUtf8("font-size: 18px; color: #666;"));

        vboxLayout1->addWidget(lblPreview);

        scrollArea->setWidget(scrollAreaWidgetContents);

        vboxLayout->addWidget(scrollArea);

        resultLabel = new QLabel(centralwidget);
        resultLabel->setObjectName("resultLabel");
        resultLabel->setMinimumSize(QSize(780, 100));
        resultLabel->setWordWrap(true);
        resultLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);

        vboxLayout->addWidget(resultLabel);

        MainWindow->setCentralWidget(centralwidget);
        QWidget::setTabOrder(btnUpload, captureButton);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        btnUpload->setText(QCoreApplication::translate("MainWindow", "\344\270\212\344\274\240\345\233\276\347\211\207", nullptr));
        captureButton->setText(QCoreApplication::translate("MainWindow", "\346\210\252\345\233\276\350\257\206\345\210\253", nullptr));
        lblPreview->setText(QCoreApplication::translate("MainWindow", "\345\260\206\345\205\254\345\274\217\345\233\276\347\211\207\346\213\226\346\224\276\345\210\260\346\255\244\345\214\272\345\237\237", nullptr));
        (void)MainWindow;
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
