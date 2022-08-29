
#include <brlcad/Object.h>
#include <brlcad/Combination.h>
#include <QMessageBox>
#include "MatrixTransformWidget.h"

QHash<int,QWidget*> MatrixTransformWidget::widgets;

MatrixTransformWidget::MatrixTransformWidget(Document *document, int childObjectId, TransformType transformType)
        : DataRow(3, true) {
    setWindowFlags( Qt::Window| Qt::WindowCloseButtonHint);
    setAttribute( Qt::WA_QuitOnClose, false );
    QString parentObjectName = document->getObjectTree()->getNameMap()[document->getObjectTree()->getParent()[childObjectId]];
    QString childNodeName = document->getObjectTree()->getNameMap()[childObjectId];
    setWindowTitle(childNodeName);
    if (parentObjectName == ""){
        QMessageBox::information(this, "Can't Transform Top Object", "You cannot transform top objects", QMessageBox::Ok);
        delete this;
        return;
    }

    if(transformType == Translate)setWindowTitle(childNodeName + +"  : Move");
    if(transformType == Rotate)setWindowTitle(childNodeName + +"  : Rotate");
    if(transformType == Scale)setWindowTitle(childNodeName + +"  : Scale");

    if(widgets.contains(childObjectId)) widgets[childObjectId]->close();
    widgets[childObjectId] = this;

    double *transformationMatrix = new double[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    document->getBRLCADConstObject(parentObjectName, [this, childNodeName, transformationMatrix](const BRLCAD::Object &object) {
        BRLCAD::Combination::ConstTreeNode tree = dynamic_cast<const BRLCAD::Combination *>(&object)->Tree();
        const double *transformationMatrixPtr = getLeafMatrix(tree, childNodeName);
        if (transformationMatrixPtr != nullptr) {
            for(int i=0;i<16;i++)transformationMatrix[i] = transformationMatrixPtr[i];
        }
    });

    for (int i = 0; i < 3; i++) {
        getTextBoxes()[i]->setText("0");
        if(transformType == Scale) getTextBoxes()[i]->setText("1");

        connect(getTextBoxes()[i], &QLineEdit::textEdited, this, [transformType, this,transformationMatrix,document,parentObjectName,childObjectId, childNodeName ]() {
            double change[3];
            change[0] = getTextBoxes()[0]->text().toDouble();
            change[1] = getTextBoxes()[1]->text().toDouble();
            change[2] = getTextBoxes()[2]->text().toDouble();

            QMatrix4x4 m = QMatrix4x4(transformationMatrix[0], transformationMatrix[1], transformationMatrix[2], transformationMatrix[3],
                                      transformationMatrix[4], transformationMatrix[5], transformationMatrix[6], transformationMatrix[7],
                                      transformationMatrix[8], transformationMatrix[9], transformationMatrix[10], transformationMatrix[11],
                                      transformationMatrix[12], transformationMatrix[13], transformationMatrix[14], transformationMatrix[15]).transposed();

            if(transformType == Translate) {
                m.translate(change[0], change[1], change[2]);
            }
            if(transformType == Scale) {
                m.scale(change[0], change[1], change[2]);
            }
            if(transformType == Rotate){
                double degToRad = 0.01745329252;
                m.rotate(change[0]*degToRad, QVector3D(1.f,0.f,0.f));
                m.rotate(change[1]*degToRad, QVector3D(0.f,1.f,0.f));
                m.rotate(change[2]*degToRad, QVector3D(0.f,0.f,1.f));
            }

            m = m.transposed();

            double *newTransformationMatrix = new double[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, .5};

            for (int i = 0; i < 16; ++i) newTransformationMatrix[i] = m.data()[i];


            document->getBRLCADObject(parentObjectName, [this, childNodeName,childObjectId, newTransformationMatrix,document](BRLCAD::Object &object) {
                BRLCAD::Combination::TreeNode tree = dynamic_cast<BRLCAD::Combination *>(&object)->Tree();
                setLeafMatrix(tree, childNodeName, newTransformationMatrix);
            });
        });
    }

    show();
}
