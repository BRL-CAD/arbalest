#include "DragWindowMouseAction.h"
#include <QMouseEvent>


DragWindowMouseAction::DragWindowMouseAction(QMainWindow* mainWindow, QMenuBar* menuTitleBar) 
    : MouseAction(menuTitleBar), m_mainWindow{mainWindow} {}

DragWindowMouseAction::~DragWindowMouseAction() {}

bool DragWindowMouseAction::eventFilter(QObject* watched, QEvent* event)
{
    static QPoint dragPosition{};
    if (watched == m_watched)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::LeftButton)
            {
                dragPosition = mouse_event->globalPos() - m_mainWindow->frameGeometry().topLeft();
                return false;
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if (mouse_event->buttons() & Qt::LeftButton)
            {
                if (m_mainWindow->isMaximized()) return false;//showNormal();
                //todo showNormal when dragged
                m_mainWindow->move(mouse_event->globalPos() - dragPosition);
                return false;
            }
        }
    }
    return false;
}
