#ifndef BASE_DEBUG_CC
#define BASE_DEBUG_CC

#include "services/debug.h"

debugSignaler *debugSignaler::instance = NULL;

debugSignaler *debugSignaler::getInstance() {
    if(instance == NULL)
        instance = new debugSignaler();
    return instance;
}

const void debugSignaler::pauseExec() {
    emit pause();
}

#endif // BASE_DEBUG_CC
