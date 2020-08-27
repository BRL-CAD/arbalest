
#include <brlcad/Object.h>
#include <brlcad/Combination.h>
#include <dependancies/glm/glm/gtc/type_ptr.hpp>
#include <QtWidgets/QMessageBox>
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
    getBRLCADObject(document->getDatabase(), parentObjectName, [this, childNodeName, transformationMatrix](BRLCAD::Object &object) {
        BRLCAD::Combination::TreeNode tree = dynamic_cast<BRLCAD::Combination *>(&object)->Tree();
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


            auto m = glm::transpose(glm::make_mat4(transformationMatrix));

            if(transformType == Translate) {
                m = glm::translate(m, glm::make_vec3(change));
            }
            if(transformType == Scale) {
                m = glm::scale(m, glm::make_vec3(change));
            }
            if(transformType == Rotate){
                double degToRad = 0.01745329252;
                m = glm::rotate(m,change[0]*degToRad, glm::make_vec3(new double[3]{1,0,0}));
                m = glm::rotate(m,change[1]*degToRad, glm::make_vec3(new double[3]{0,1,0}));
                m = glm::rotate(m,change[2]*degToRad, glm::make_vec3(new double[3]{0,0,1}));
            }

            m = glm::transpose(m);

            double *newTransformationMatrix = new double[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, .5};

            for (int i = 0; i < 16; ++i) newTransformationMatrix[i] = glm::value_ptr(m)[i];


            getBRLCADObject(document->getDatabase(), parentObjectName, [this, childNodeName,childObjectId, newTransformationMatrix,document](BRLCAD::Object &object) {
                BRLCAD::Combination::TreeNode tree = dynamic_cast<BRLCAD::Combination *>(&object)->Tree();
                setLeafMatrix(tree, childNodeName, newTransformationMatrix);
            });
            document->modifyObjectNoSet(document->getObjectTree()->getParent()[childObjectId]);
        });
    }

    show();
}
