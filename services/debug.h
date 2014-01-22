/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */

#ifndef BASE_DEBUG_H
#define BASE_DEBUG_H

#include <assert.h>
#include <QObject>

#define AssertNotReached() assert(0)

class debugSignaler : public QObject{
    Q_OBJECT

signals:
    void pause();

private:
    static debugSignaler *instance;

    debugSignaler() {}

public:
    ~debugSignaler() {}
    static debugSignaler *getInstance();
    const void pauseExec();
};

#endif // BASE_DEBUG_H
