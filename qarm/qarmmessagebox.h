#ifndef QARMMESSAGEBOX_H
#define QARMMESSAGEBOX_H

#include <QObject>
#include <QDialog>

class QarmMessageBox : public QDialog
{
public:
    typedef enum {
        INFO,
        WARNING,
        CRITICAL,
        QUESTION
    } DType;

    QarmMessageBox( DType t, QString &title, QString &text,
                    QWidget * parent = 0, Qt::WindowFlags f = 0 );

    QarmMessageBox( DType t, const char *title, const char *text,
                    QWidget * parent = 0, Qt::WindowFlags f = 0 );
};

#endif // QARMMESSAGEBOX_H
