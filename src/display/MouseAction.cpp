#include "MouseAction.h"


MouseAction::MouseAction(QObject* watched, QWidget* widget)
	: m_watched{watched}, m_widget{widget} {}

MouseAction::~MouseAction() {
	m_watched = nullptr;
	m_widget = nullptr;
}

void MouseAction::WatchedDestroyed(QObject* watched) {}