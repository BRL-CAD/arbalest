#include "MouseAction.h"


MouseAction::MouseAction(QObject* watched)
	: m_watched{watched} {}

MouseAction::~MouseAction() {}

void MouseAction::WatchedDestroyed(QObject* watched) {}