/********************************************************************************
** Form generated from reading UI file 'overlays.ui'
**
** Created by: Qt User Interface Compiler version 5.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef OVERLAYS_H
#define OVERLAYS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Form
{
public:
    QVBoxLayout *verticalLayout_3;
    QTabWidget *tabWidget_overlays;
    QWidget *tab;
    QVBoxLayout *verticalLayout_2;
    QFormLayout *formLayout;
    QLabel *label_5;
    QLabel *label;
    QSlider *horizontalSlider_window;
    QSpacerItem *verticalSpacer;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_4;
    QFormLayout *formLayout_4;
    QLabel *label_8;
    QLabel *label_9;
    QSlider *horizontalSlider_askedge;
    QSpacerItem *verticalSpacer_3;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout_2;
    QLabel *label_2;
    QLabel *label_6;
    QSlider *horizontalSlider_dirthr_up;
    QFormLayout *formLayout_3;
    QLabel *label_3;
    QLabel *label_7;
    QSlider *horizontalSlider_dirthr_down;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_apply;
    QPushButton *pushButton_sticky;
    QSpacerItem *horizontalSpacer;
    QLabel *label_4;

    void setupUi(QWidget *Form)
    {
        if (Form->objectName().isEmpty())
            Form->setObjectName(QStringLiteral("Form"));
        Form->resize(614, 286);
        verticalLayout_3 = new QVBoxLayout(Form);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        tabWidget_overlays = new QTabWidget(Form);
        tabWidget_overlays->setObjectName(QStringLiteral("tabWidget_overlays"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        tab->setFocusPolicy(Qt::StrongFocus);
        verticalLayout_2 = new QVBoxLayout(tab);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label_5 = new QLabel(tab);
        label_5->setObjectName(QStringLiteral("label_5"));

        formLayout->setWidget(0, QFormLayout::FieldRole, label_5);

        label = new QLabel(tab);
        label->setObjectName(QStringLiteral("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);


        verticalLayout_2->addLayout(formLayout);

        horizontalSlider_window = new QSlider(tab);
        horizontalSlider_window->setObjectName(QStringLiteral("horizontalSlider_window"));
        horizontalSlider_window->setMinimum(10);
        horizontalSlider_window->setMaximum(10000);
        horizontalSlider_window->setValue(2000);
        horizontalSlider_window->setOrientation(Qt::Horizontal);

        verticalLayout_2->addWidget(horizontalSlider_window);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        tabWidget_overlays->addTab(tab, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        verticalLayout_4 = new QVBoxLayout(tab_3);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        formLayout_4 = new QFormLayout();
        formLayout_4->setObjectName(QStringLiteral("formLayout_4"));
        formLayout_4->setContentsMargins(-1, -1, -1, 0);
        label_8 = new QLabel(tab_3);
        label_8->setObjectName(QStringLiteral("label_8"));

        formLayout_4->setWidget(0, QFormLayout::LabelRole, label_8);

        label_9 = new QLabel(tab_3);
        label_9->setObjectName(QStringLiteral("label_9"));

        formLayout_4->setWidget(0, QFormLayout::FieldRole, label_9);


        verticalLayout_4->addLayout(formLayout_4);

        horizontalSlider_askedge = new QSlider(tab_3);
        horizontalSlider_askedge->setObjectName(QStringLiteral("horizontalSlider_askedge"));
        horizontalSlider_askedge->setMinimum(5);
        horizontalSlider_askedge->setMaximum(80);
        horizontalSlider_askedge->setValue(20);
        horizontalSlider_askedge->setOrientation(Qt::Horizontal);

        verticalLayout_4->addWidget(horizontalSlider_askedge);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_3);

        tabWidget_overlays->addTab(tab_3, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        verticalLayout = new QVBoxLayout(tab_2);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        formLayout_2 = new QFormLayout();
        formLayout_2->setObjectName(QStringLiteral("formLayout_2"));
        label_2 = new QLabel(tab_2);
        label_2->setObjectName(QStringLiteral("label_2"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_2);

        label_6 = new QLabel(tab_2);
        label_6->setObjectName(QStringLiteral("label_6"));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, label_6);


        verticalLayout->addLayout(formLayout_2);

        horizontalSlider_dirthr_up = new QSlider(tab_2);
        horizontalSlider_dirthr_up->setObjectName(QStringLiteral("horizontalSlider_dirthr_up"));
        horizontalSlider_dirthr_up->setMaximum(128);
        horizontalSlider_dirthr_up->setValue(20);
        horizontalSlider_dirthr_up->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(horizontalSlider_dirthr_up);

        formLayout_3 = new QFormLayout();
        formLayout_3->setObjectName(QStringLiteral("formLayout_3"));
        label_3 = new QLabel(tab_2);
        label_3->setObjectName(QStringLiteral("label_3"));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, label_3);

        label_7 = new QLabel(tab_2);
        label_7->setObjectName(QStringLiteral("label_7"));

        formLayout_3->setWidget(0, QFormLayout::FieldRole, label_7);


        verticalLayout->addLayout(formLayout_3);

        horizontalSlider_dirthr_down = new QSlider(tab_2);
        horizontalSlider_dirthr_down->setObjectName(QStringLiteral("horizontalSlider_dirthr_down"));
        horizontalSlider_dirthr_down->setMaximum(127);
        horizontalSlider_dirthr_down->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(horizontalSlider_dirthr_down);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        tabWidget_overlays->addTab(tab_2, QString());

        verticalLayout_3->addWidget(tabWidget_overlays);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        pushButton_apply = new QPushButton(Form);
        pushButton_apply->setObjectName(QStringLiteral("pushButton_apply"));

        horizontalLayout->addWidget(pushButton_apply);

        pushButton_sticky = new QPushButton(Form);
        pushButton_sticky->setObjectName(QStringLiteral("pushButton_sticky"));

        horizontalLayout->addWidget(pushButton_sticky);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_4 = new QLabel(Form);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout->addWidget(label_4);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(Form);
        QObject::connect(horizontalSlider_window, SIGNAL(valueChanged(int)), label_4, SLOT(setNum(int)));
        QObject::connect(horizontalSlider_window, SIGNAL(valueChanged(int)), label_5, SLOT(setNum(int)));
        QObject::connect(horizontalSlider_dirthr_up, SIGNAL(valueChanged(int)), label_6, SLOT(setNum(int)));
        QObject::connect(horizontalSlider_dirthr_down, SIGNAL(valueChanged(int)), label_7, SLOT(setNum(int)));
        QObject::connect(horizontalSlider_askedge, SIGNAL(valueChanged(int)), label_9, SLOT(setNum(int)));

        tabWidget_overlays->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(Form);
    } // setupUi

    void retranslateUi(QWidget *Form)
    {
        Form->setWindowTitle(QApplication::translate("Form", "Overlays", 0));
        label_5->setText(QString());
        label->setText(QApplication::translate("Form", "Window size", 0));
        tabWidget_overlays->setTabText(tabWidget_overlays->indexOf(tab), QApplication::translate("Form", "Autocorrelate", 0));
        label_8->setText(QApplication::translate("Form", "Edge Jump Threshold", 0));
        label_9->setText(QString());
        tabWidget_overlays->setTabText(tabWidget_overlays->indexOf(tab_3), QApplication::translate("Form", "AskEdge", 0));
        label_2->setText(QApplication::translate("Form", "Up", 0));
        label_6->setText(QString());
        label_3->setText(QApplication::translate("Form", "Down", 0));
        label_7->setText(QString());
        tabWidget_overlays->setTabText(tabWidget_overlays->indexOf(tab_2), QApplication::translate("Form", "Dirthreshold", 0));
        pushButton_apply->setText(QApplication::translate("Form", "Apply", 0));
        pushButton_sticky->setText(QApplication::translate("Form", "Restore", 0));
        label_4->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Form: public Ui_Form {};
} // namespace Ui

QT_END_NAMESPACE

#endif // OVERLAYS_H
