
#include <QtWidgets/QStyledItemDelegate>
#include <QtCore/QtCore>
#include <QApplication>
#include <QMouseEvent>
#include <QRect>
#include <QPainter>
#include <iostream>
#include "MainWindow.h"
#include <include/ObjectTreeRowButtons.h>
#include "Globals.h"


ObjectTreeRowButtons::ObjectTreeRowButtons(ObjectTree *objectTree, QObject *parent) : objectTree(objectTree), QStyledItemDelegate(parent) {
    iconFullVisible             = QImage(visibilityIconFilePath);
    iconSomeChildrenVisible     = QImage(visibilityIconFilePath);
    iconInvisible               = QImage(visibilityIconFilePath);
    iconCenter                  = QImage(centerIconFilePath);

    for (int y = 0; y < iconSomeChildrenVisible.height(); y++) {
        for (int x = 0; x < iconSomeChildrenVisible.width()/2; x++) {
            QColor pixel = iconSomeChildrenVisible.pixelColor(x, y);
            pixel.setBlue(220);
            pixel.setGreen(220);
            pixel.setRed(220);
            iconSomeChildrenVisible.setPixelColor(x, y, pixel);
        }
        for (int x = iconSomeChildrenVisible.width()/2; x < iconSomeChildrenVisible.width(); x++) {
            QColor pixel = iconSomeChildrenVisible.pixelColor(x, y);
            pixel.setBlue(255);
            pixel.setGreen(0);
            pixel.setRed(0);
            iconSomeChildrenVisible.setPixelColor(x, y, pixel);
        }
    }

    for (int y = 0; y < iconFullVisible.height(); y++) {
        for (int x = 0; x < iconFullVisible.width(); x++) {
            QColor pixel = iconFullVisible.pixelColor(x, y);
            pixel.setBlue(255);
            pixel.setGreen(0);
            pixel.setRed(0);
            iconFullVisible.setPixelColor(x, y, pixel);
        }
    }

    for (int y = 0; y < iconInvisible.height(); y++) {
        for (int x = 0; x < iconInvisible.width(); x++) {
            QColor pixel = iconInvisible.pixelColor(x, y);
            pixel.setBlue(170);
            pixel.setGreen(170);
            pixel.setRed(170);
            iconInvisible.setPixelColor(x, y, pixel);
        }
    }
    
    for (int y = 0; y < iconCenter.height(); y++) {
        for (int x = 0; x < iconCenter.width(); x++) {
            QColor pixel = iconCenter.pixelColor(x, y);
            pixel.setBlue(170);
            pixel.setGreen(170);
            pixel.setRed(170);
            iconCenter.setPixelColor(x, y, pixel);
        }
    }
}

QPoint ObjectTreeRowButtons::visibilityIconPosition(const QStyleOptionViewItem &option) const {
    return {        option.rect.right() - iconFullVisible.width() - margin-5,
                    option.rect.center().y() - iconFullVisible.height() / 2};
}
QPoint ObjectTreeRowButtons::centerIconPosition(const QStyleOptionViewItem &option) const {
    return {        option.rect.right() - iconFullVisible.width() - iconCenter.width() - margin-10,
                    option.rect.center().y() - iconFullVisible.height() / 2};
}

void ObjectTreeRowButtons::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
    if (option.state & QStyle::State_MouseOver) {
        switch (visibilityState){
            case ObjectTree::Invisible:
                painter->drawImage(visibilityIconPosition(option), iconInvisible);
                break;
            case ObjectTree::SomeChildrenVisible:
                painter->drawImage(visibilityIconPosition(option), iconSomeChildrenVisible);
                break;
            case ObjectTree::FullyVisible:
                painter->drawImage(visibilityIconPosition(option), iconFullVisible);
                break;
        }
        painter->drawImage(centerIconPosition(option), iconCenter);
    }
}

QSize ObjectTreeRowButtons::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.rwidth() += iconFullVisible.width() + iconCenter.width() + 10+ margin * 2;
    size.setHeight(qMax(size.height(), iconFullVisible.height() + margin * 2));
    return size;
}

bool ObjectTreeRowButtons::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    objectId = index.data(Qt::UserRole).toInt();
    visibilityState = objectTree->getObjectVisibility()[objectId];
    QRect visibilityIconRect = iconFullVisible.rect().translated(visibilityIconPosition(option));
    QRect centerIconRect = iconFullVisible.rect().translated(centerIconPosition(option));
    // Emit a signal when the icon is clicked
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (visibilityIconRect.contains(mouseEvent->pos())) {
            emit visibilityButtonClicked(objectId);
            visibilityState = objectTree->getObjectVisibility()[objectId];
            return true;
        }
        else if (centerIconRect.contains(mouseEvent->pos())) {
            emit centerButtonClicked(objectId);
            return true;
        }
    }
    else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (visibilityIconRect.contains(mouseEvent->pos())) {
            Globals::mainWindow->getStatusBar()->showMessage("Change object visibility", Globals::mainWindow->statusBarShortMessageDuration);
            return true;
        }
        else if (centerIconRect.contains(mouseEvent->pos())) {
            Globals::mainWindow->getStatusBar()->showMessage("Focus on object",  Globals::mainWindow->statusBarShortMessageDuration);
            return true;
        }
    }
    return false;
}

