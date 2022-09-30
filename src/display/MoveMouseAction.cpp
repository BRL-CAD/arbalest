#include "MoveMouseAction.h"

MoveMouseAction::MoveMouseAction(QObject* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

MoveMouseAction::~MoveMouseAction() {}

bool MoveMouseAction::eventFilter(QObject* watched, QEvent* event) {}