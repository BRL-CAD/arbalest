#include <cmath>
#include <QString>
#include <include/Utils.h>
#include <brlcad/Database/Combination.h>
#include <QInputDialog>
#include <QMessageBox>
#include "Globals.h"

BRLCAD::Vector3D operator+(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b)
{
    BRLCAD::Vector3D result;
    result.coordinates[0] = a.coordinates[0] + b.coordinates[0];
    result.coordinates[1] = a.coordinates[1] + b.coordinates[1];
    result.coordinates[2] = a.coordinates[2] + b.coordinates[2];
    return result;
}

BRLCAD::Vector3D operator-(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b)
{
    BRLCAD::Vector3D result;
    result.coordinates[0] = a.coordinates[0] - b.coordinates[0];
    result.coordinates[1] = a.coordinates[1] - b.coordinates[1];
    result.coordinates[2] = a.coordinates[2] - b.coordinates[2];
    return result;
}

BRLCAD::Vector3D operator/(const BRLCAD::Vector3D& a, const int& b)
{
    BRLCAD::Vector3D result;
    result.coordinates[0] = a.coordinates[0] / b;
    result.coordinates[1] = a.coordinates[1] / b;
    result.coordinates[2] = a.coordinates[2] / b;
    return result;
}

double vector3DLength(const BRLCAD::Vector3D& a)
{
    return sqrt(a.coordinates[0] * a.coordinates[0] + a.coordinates[1] * a.coordinates[1] + a.coordinates[2] * a.coordinates[2]);
}

QString breakStringAtCaps(const QString& in)
{
    QString newName;
    for (int i = 0; i < in.size(); i++)
    {
        if (in[i].isUpper() && i != 0) newName += " ";
        newName += in[i];
    }
    return newName;
}

const double * getLeafMatrix(BRLCAD::Combination::ConstTreeNode& node, const QString& name) {
    switch (node.Operation())
    {
        case BRLCAD::Combination::ConstTreeNode::Union:
        case BRLCAD::Combination::ConstTreeNode::Intersection:
        case BRLCAD::Combination::ConstTreeNode::Subtraction:
        case BRLCAD::Combination::ConstTreeNode::ExclusiveOr: {
            BRLCAD::Combination::ConstTreeNode left = node.LeftOperand();
            BRLCAD::Combination::ConstTreeNode right = node.RightOperand();
            const double * resultLeft = getLeafMatrix(left, name);
            const double * resultRight = getLeafMatrix(right, name);
            if (resultLeft) return resultLeft;
            return resultRight;
        }

        case BRLCAD::Combination::ConstTreeNode::Not: {
            BRLCAD::Combination::ConstTreeNode op = node.RightOperand();
            return getLeafMatrix(op, name);
        }

        case BRLCAD::Combination::ConstTreeNode::Leaf: {
            QString leafName = QString(node.Name());
            if (leafName == name) {
                return node.Matrix();
            }
            return nullptr;
        }

        case BRLCAD::Combination::ConstTreeNode::Null: {
            return nullptr;
        }
    }

    return nullptr;
}

void setLeafMatrix(BRLCAD::Combination::TreeNode& node, const QString& name, double * matrix) {
    switch (node.Operation())
    {
        case BRLCAD::Combination::ConstTreeNode::Union:
        case BRLCAD::Combination::ConstTreeNode::Intersection:
        case BRLCAD::Combination::ConstTreeNode::Subtraction:
        case BRLCAD::Combination::ConstTreeNode::ExclusiveOr: {
            BRLCAD::Combination::TreeNode left = node.LeftOperand();
            BRLCAD::Combination::TreeNode right = node.RightOperand();
            setLeafMatrix(left, name, matrix);
            setLeafMatrix(right, name, matrix);
            break;
        }

        case BRLCAD::Combination::ConstTreeNode::Not: {
            BRLCAD::Combination::TreeNode op = node.RightOperand();
            setLeafMatrix(op, name, matrix);
            break;
        }

        case BRLCAD::Combination::ConstTreeNode::Leaf: {
            QString leafName = QString(node.Name());
            if (leafName == name) node.SetMatrix(matrix);
            break;
        }

        case BRLCAD::Combination::ConstTreeNode::Null: {
        }
    }
}

QImage coloredIcon(QString path, QString colorKey){
    QColor color;
    if (colorKey == ""){
        color = QColor(0, 0, 0);
    }else {
        color = Globals::theme->getColor(colorKey);
    }

    QImage oldImage =  QImage(path);
    QImage image = QImage(oldImage.size(),QImage::Format_ARGB32);

    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QColor pixel = oldImage.pixelColor(x, y);
            pixel.setRed(color.red());
            pixel.setGreen(color.green());
            pixel.setBlue(color.blue());
            image.setPixelColor(x, y, pixel);
        }
    }
    return image;
}

bool getObjectNameFromUser(QWidget* parent, Document& document, QString& name) {
    bool ok;

    while (true) {
        name = QInputDialog::getText(parent, QObject::tr("Object Name"), QObject::tr("Enter object name"), QLineEdit::Normal, "", &ok);

        if (ok) {
            if (name.isEmpty()) {
                QMessageBox::information(parent, QObject::tr("Object Name"), QObject::tr("Please enter an object name"), QMessageBox::Ok);
            }
            else if (document.getDatabase()->Get(name.toUtf8().data())) {
                QMessageBox::information(parent, QObject::tr("Object Name"), QObject::tr("Please enter an unique object name"), QMessageBox::Ok);
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }

    return ok;
}
