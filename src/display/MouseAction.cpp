#include "MouseAction.h"

MouseAction::MouseAction(QObject* parent, Display* watched)
    : QObject{parent}, m_watched{watched} {}

MouseAction::~MouseAction() {
    m_watched = nullptr;
}

void MouseAction::WatchedDestroyed(QObject* watched) {}