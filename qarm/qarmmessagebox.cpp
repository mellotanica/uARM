#include "qarmmessagebox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QDialogButtonBox>
#include <QStyle>
#include <QApplication>

QarmMessageBox::QarmMessageBox(DType t, QString &title, QString &text, QWidget * parent, Qt::WindowFlags f)
{
    QarmMessageBox(t, title.toStdString().c_str(), text.toStdString().c_str(), parent, f);
}

QarmMessageBox::QarmMessageBox(DType t, const char *title, const char *text, QWidget * parent, Qt::WindowFlags f):
    QDialog(parent,f)
{
    setModal(true);
    setWindowTitle(title);

    QLayout *mainLayout = new QVBoxLayout;
    QWidget *topWidget = new QWidget;

    QLayout *topLayout = new QHBoxLayout;
    QStyle *style = QApplication::style();
    QIcon icon;
    QString accName;
    switch(t){
        case INFO:
            icon = style->standardIcon(QStyle::SP_MessageBoxInformation);
            accName = QString("Information Dialog");
            break;
        case WARNING:
            icon = style->standardIcon(QStyle::SP_MessageBoxWarning);
            accName = QString("Warning Dialog");
            break;
        case CRITICAL:
            icon = style->standardIcon(QStyle::SP_MessageBoxCritical);
            accName = QString("Error Dialog");
            break;
        case QUESTION:
            icon = style->standardIcon(QStyle::SP_MessageBoxQuestion);
            accName = QString("Question Dialog");
            break;
    }
    if(parent)
        setWindowIcon(parent->windowIcon());
    else
        setWindowIcon(icon);

    this->setAccessibleName(accName);

    QLabel *iconL = new QLabel(this);
    iconL->setAlignment(Qt::AlignCenter);
    //iconL->setFrameShape(QFrame::Box);
    iconL->setAccessibleName(accName);
    iconL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    iconL->setMinimumSize(64, 64);
    iconL->setPixmap(icon.pixmap(256, 256, QIcon::Normal, QIcon::On));
    topLayout->addWidget(iconL);

    QLabel *contentL = new QLabel(text, this);
    contentL->setAccessibleName("Dialog content");
    contentL->setAccessibleDescription(text);

    topLayout->addWidget(contentL);

    topWidget->setLayout(topLayout);

    QDialogButtonBox *buttonBox;
    if(t == QUESTION){
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    } else {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    }

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mainLayout->addWidget(topWidget);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}
