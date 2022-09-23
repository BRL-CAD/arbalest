#include "MouseAction.h"


MouseAction::MouseAction(QWidget* widget, QObject* watched)
	: QObject {watched}, m_watched {widget} {}

MouseAction::~MouseAction() {
	m_watched = nullptr;
}

void MouseAction::WatchedDestroyed(QObject* watched) {}