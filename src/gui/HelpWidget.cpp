
#include <QtWidgets/QLabel>
#include <QBitmap>
#include <QIcon>
#include <QDesktopWidget>
#include <QApplication>
#include <QtWidgets/QScrollArea>
#include <algorithm>
#include <iostream>
#include "HelpWidget.h"
#include "Globals.h"

HelpWidget::HelpWidget() : QVBoxWidget() {

    setObjectName("helpWidget");

    QVBoxWidget *container = new QVBoxWidget();

    QString
    text = "New File  <a href=\"Ctrl+N\" style=\"color:$Color-ColorText\">Ctrl+N</a> <br><br>"
           "Open File  <a href=\"Ctrl+O\" style=\"color:$Color-ColorText\">Ctrl+O</a> <br><br>"
           "Save File  <a href=\"Ctrl+S\" style=\"color:$Color-ColorText\">Ctrl+S</a> <br><br>"
           "<br><br>"
           "Drag with <font style=\"color:$Color-ColorText\">Mouse Left Button</font> to rotate viewport camera<br><br>"
           "Drag with <font style=\"color:$Color-ColorText\">Mouse Right Button</font> to move viewport camera<br><br>"
           "Zoom In / Zoom Out <font style=\"color:$Color-ColorText\">Mouse Wheel</font> <br><br>"
           "<br><br>"
           "Focus camera on selected item <font style=\"color:$Color-ColorText\">F</font> <br><br>"
           "Focus camera on all visible items <font style=\"color:$Color-ColorText\">Ctrl+F</font> <br><br>"
           "Use keys <font style=\"color:$Color-ColorText\">1 2 3 4</font> to switch between viewports<br><br>"
           "Open all viewports in quad view <font style=\"color:$Color-ColorText\">5</font> <br><br>"
           "Toggle grid on / off <font style=\"color:$Color-ColorText\">G</font> <br><br>"
           "<br><br>"
           "Raytrace current viewport <font style=\"color:$Color-ColorText\">Ctrl+R</font> <br><br>"

           ;

    text = Globals::theme->process(text);

    QLabel *intro = new QLabel(text);
    //intro->setWordWrap(true);
    intro->setMargin(5);
    intro->setObjectName("helpWidget");
    intro->setTextFormat(Qt::RichText);
    intro->setTextInteractionFlags(Qt::TextBrowserInteraction);
    intro->setOpenExternalLinks(true);
    intro->setMargin(80);

    // Connect the link clicked signal to a slot
    connect(intro, SIGNAL(linkActivated(QString)), this, SLOT(onLinkClicked(QString)));

    container->addWidget(intro);
    QScrollArea * scrollArea = new QScrollArea();
    scrollArea->setObjectName("helpWidget");
    addWidget(scrollArea);
    scrollArea->setWidget(container);


}

void HelpWidget::onLinkClicked(const QString& link) {
    if (link == "Ctrl+N") {
        emit ctrlNPressed();
    }
}


