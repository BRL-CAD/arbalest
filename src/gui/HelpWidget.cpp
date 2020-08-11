
#include <QtWidgets/QLabel>
#include <QBitmap>
#include <QIcon>
#include <QDesktopWidget>
#include <QApplication>
#include <QtWidgets/QScrollArea>
#include <algorithm>
#include "HelpWidget.h"

HelpWidget::HelpWidget() : QVBoxWidget() {

    setObjectName("helpWidget");

    QVBoxWidget *container = new QVBoxWidget();

    QString
    text = "New File  <font style=\"color:#6666ff\">Ctrl+N</font> <br><br>"
           "Open File  <font style=\"color:#6666ff\">Ctrl+O</font> <br><br>"
           "Save File  <font style=\"color:#6666ff\">Ctrl+S</font> <br><br>"
           "<br><br>"
           "Drag with <font style=\"color:#6666ff\">Mouse Left Button</font> to rotate viewport camera<br><br>"
           "Drag with <font style=\"color:#6666ff\">Mouse Right Button</font> to move viewport camera<br><br>"
           "Zoom In / Zoom Out <font style=\"color:#6666ff\">Mouse Wheel</font> <br><br>"
           "<br><br>"
           "Focus camera on selected item <font style=\"color:#6666ff\">F</font> <br><br>"
           "Focus camera on all visible items <font style=\"color:#6666ff\">Ctrl+F</font> <br><br>"
           "Use keys <font style=\"color:#6666ff\">1 2 3 4</font> to switch between viewports<br><br>"
           "Open all viewports in quad view <font style=\"color:#6666ff\">5</font> <br><br>"
           "Toggle grid on / off <font style=\"color:#6666ff\">G</font> <br><br>"
           "<br><br>"
           "Raytrace current viewport <font style=\"color:#6666ff\">Ctrl+R</font> <br><br>"

           ;



    QLabel *intro = new QLabel(text);
    //intro->setWordWrap(true);
    intro->setMargin(5);
    intro->setObjectName("helpWidget");
    intro->setTextFormat(Qt::RichText);
    intro->setTextInteractionFlags(Qt::TextBrowserInteraction);
    intro->setOpenExternalLinks(true);
    intro->setMargin(80);

    container->addWidget(intro);
    QScrollArea * scrollArea = new QScrollArea();
    scrollArea->setObjectName("helpWidget");
    addWidget(scrollArea);
    scrollArea->setWidget(container);


}
