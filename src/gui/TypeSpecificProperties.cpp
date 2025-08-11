
#include <brlcad/Database/Cone.h>
#include <brlcad/Database/Arb8.h>


#include <QCheckBox>
#include "TypeSpecificProperties.h"
#include "QHBoxWidget.h"

#include <QStyledItemDelegate>
#include <include/QVBoxWidget.h>
#include <include/DataRow.h>
#include <include/ObjectDataField.h>
#include <include/ObjectDataTable.h>
#include <brlcad/Database/BagOfTriangles.h>
#include <brlcad/Database/EllipticalTorus.h>
#include <brlcad/Database/Halfspace.h>
#include <brlcad/Database/HyperbolicCylinder.h>
#include <brlcad/Database/Hyperboloid.h>
#include <brlcad/Database/ParabolicCylinder.h>
#include <brlcad/Database/Paraboloid.h>
#include <brlcad/Database/Particle.h>
#include <brlcad/Database/Sphere.h>
#include <brlcad/Database/Torus.h>
#include <iostream>

using namespace std;



TypeSpecificProperties::TypeSpecificProperties(Document &document, BRLCAD::Object *object, const size_t objectId)
        : document(document), object(object) {
    setObjectName("properties-TypeSpecificProperties");
    l = getBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    const QStringList pointsIndices = {"P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8"};
    const QStringList abcdIndices = {"A", "B", "C", "D", "E", "F"};

    if(QString(object->Type()) == "Combination") {
        BRLCAD::Combination *comb = dynamic_cast<BRLCAD::Combination*>(object);

        CollapsibleWidget *childrenListCollapsible = new CollapsibleWidget();
        l->addWidget(childrenListCollapsible);
        QVBoxWidget * childrenList = new QVBoxWidget();
        childrenListCollapsible->setTitle("Children");
        childrenListCollapsible->setWidget(childrenList);

        for (ObjectTreeItem *child : document.getObjectTree()->getItems()[objectId]->getChildren()){
            childrenList->addWidget(new QLabel(child->getName()));
        }

        QCheckBox *hasColorCheck = new QCheckBox();
        QHBoxWidget * colorHolder = new QHBoxWidget(this,hasColorCheck);
        l->addWidget(colorHolder);
        colorHolder->setStyleSheet("margin-top:11px;");
        hasColorCheck->setText("Has Color");
        hasColorCheck->setCheckState(comb->HasColor() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        connect(hasColorCheck,&QCheckBox::stateChanged,[this,objectId](int newState){
            this->document.getBRLCADObject(this->document.getObjectTree()->getItems()[objectId]->getPath(),[newState](BRLCAD::Object &object){
                if(newState == Qt::CheckState::Checked){
                    dynamic_cast<BRLCAD::Combination&>(object).SetHasColor(true);
                }
                else {
                    dynamic_cast<BRLCAD::Combination&>(object).SetHasColor(false);
                }
            });
        });

        colorHolder->getBoxLayout()->addStretch();

        QPushButton *colorButton = new QPushButton();
        colorButton->setObjectName("colorButton");
        colorButton->setStyleSheet("background-color:"+document.getObjectTree()->getItems()[objectId]->getColorInfo().toHexString());
        colorHolder->addWidget(colorButton);
        /*connect(colorButton, &QPushButton::clicked, this, [this,objectId](){
            const QColor &initial = this->document.getObjectTree()->getColorMap()[objectId].toQColor();
            QColor selectedColor = QColorDialog::getColor(initial);

            getBRLCADObject(this->document.getDatabase(),this->document.getObjectTree()->getFullPathMap()[objectId],[this,selectedColor](BRLCAD::Object &object){
                BRLCAD::Combination editableComb =  dynamic_cast<BRLCAD::Combination&>(object);
                editableComb.SetRed(selectedColor.redF());
                editableComb.SetGreen(selectedColor.greenF());
                editableComb.SetBlue(selectedColor.blueF());
                std::cout<<selectedColor.redF()<<" "<<selectedColor.greenF()<<" "<<selectedColor.blueF()<<std::endl;
                this->document.getDatabase()->Set(editableComb);
            });
            this->document.getObjectTree()->buildColorMap(objectId);
            this->document.modifyObjectNoSet(objectId);
            this->document.getViewportGrid()->forceRerenderAllViewports();
        });*/
    }

    if(QString(object->Type()) == "Arb8") {
        ObjectDataField<BRLCAD::Arb8> * property;
        property = new ObjectDataField<BRLCAD::Arb8>(
                &document,
                object,
                &BRLCAD::Arb8::Point,
                &BRLCAD::Arb8::SetPoint,
                1,
                dynamic_cast<BRLCAD::Arb8*>(object)->NumberOfVertices(),
                pointsIndices,
                "Points");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "Cone") {
        ObjectDataField<BRLCAD::Cone> * property;

        property = new ObjectDataField<BRLCAD::Cone>(
                &document,
                object,
                &BRLCAD::Cone::BasePoint,
                &BRLCAD::Cone::SetBasePoint,
                "Base Point");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Cone>(
                &document,
                object,
                &BRLCAD::Cone::Height,
                &BRLCAD::Cone::SetHeight,
                "Height");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Cone>(
                &document,
                object,
                &BRLCAD::Cone::SemiPrincipalAxis,
                &BRLCAD::Cone::SetSemiPrincipalAxis,
                0,
                3,
                abcdIndices,
                "Semi Principal Axes");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "Ellipsoid") {
        ObjectDataField<BRLCAD::Ellipsoid> * property;

        property = new ObjectDataField<BRLCAD::Ellipsoid>(
                &document,
                object,
                &BRLCAD::Ellipsoid::Center,
                &BRLCAD::Ellipsoid::SetCenter,
                "Center");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Ellipsoid>(
                &document,
                object,
                &BRLCAD::Ellipsoid::SemiPrincipalAxis,
                &BRLCAD::Ellipsoid::SetSemiPrincipalAxis,
                0,
                3,
                abcdIndices,
                "Semi Principal Axes");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "EllipticalTorus") {
        ObjectDataField<BRLCAD::EllipticalTorus> * property;

        property = new ObjectDataField<BRLCAD::EllipticalTorus>(
                &document,
                object,
                &BRLCAD::EllipticalTorus::Center,
                &BRLCAD::EllipticalTorus::SetCenter,
                "Center");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::EllipticalTorus>(
                &document,
                object,
                &BRLCAD::EllipticalTorus::Normal,
                &BRLCAD::EllipticalTorus::SetNormal,
                "Normal");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::EllipticalTorus>(
                &document,
                object,
                &BRLCAD::EllipticalTorus::TubeCenterLineRadius,
                &BRLCAD::EllipticalTorus::SetTubeCenterLineRadius,
                "Tube Center Line Radius");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::EllipticalTorus>(
                &document,
                object,
                &BRLCAD::EllipticalTorus::TubeSemiMajorAxis,
                &BRLCAD::EllipticalTorus::SetTubeSemiMajorAxis,
                "Tube Semi Major Axes");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::EllipticalTorus>(
                &document,
                object,
                &BRLCAD::EllipticalTorus::TubeSemiMinorAxis,
                &BRLCAD::EllipticalTorus::SetTubeSemiMinorAxis,
                "Tube Semi Minor Axes");
        l->addWidget(property);

    }


    if(QString(object->Type()) == "Halfspace") {
        ObjectDataField<BRLCAD::Halfspace> * property;

        property = new ObjectDataField<BRLCAD::Halfspace>(
                &document,
                object,
                &BRLCAD::Halfspace::Normal,
                &BRLCAD::Halfspace::SetNormal,
                "Normal");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Halfspace>(
                &document,
                object,
                &BRLCAD::Halfspace::DistanceFromOrigin,
                &BRLCAD::Halfspace::SetDistanceFromOrigin,
                "Distance From Origin");
        l->addWidget(property);

    }


    if(QString(object->Type()) == "HyperbolicCylinder") {
        ObjectDataField<BRLCAD::HyperbolicCylinder> * property;

        property = new ObjectDataField<BRLCAD::HyperbolicCylinder>(
                &document,
                object,
                &BRLCAD::HyperbolicCylinder::BasePoint,
                &BRLCAD::HyperbolicCylinder::SetBasePoint,
                "Base Point");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::HyperbolicCylinder>(
                &document,
                object,
                &BRLCAD::HyperbolicCylinder::Height,
                &BRLCAD::HyperbolicCylinder::SetHeight,
                "Height");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::HyperbolicCylinder>(
                &document,
                object,
                &BRLCAD::HyperbolicCylinder::Depth,
                &BRLCAD::HyperbolicCylinder::SetDepth,
                "Depth");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::HyperbolicCylinder>(
                &document,
                object,
                &BRLCAD::HyperbolicCylinder::HalfWidth,
                &BRLCAD::HyperbolicCylinder::SetHalfWidth,
                "Half Width");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::HyperbolicCylinder>(
                &document,
                object,
                &BRLCAD::HyperbolicCylinder::ApexAsymptoteDistance,
                &BRLCAD::HyperbolicCylinder::SetApexAsymptoteDistance,
                "Apex Asymptote Distance");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "Hyperboloid") {
        ObjectDataField<BRLCAD::Hyperboloid> * property;

        property = new ObjectDataField<BRLCAD::Hyperboloid>(
                &document,
                object,
                &BRLCAD::Hyperboloid::BasePoint,
                &BRLCAD::Hyperboloid::SetBasePoint,
                "Base Point");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Hyperboloid>(
                &document,
                object,
                &BRLCAD::Hyperboloid::Height,
                &BRLCAD::Hyperboloid::SetHeight,
                "Height");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Hyperboloid>(
                &document,
                object,
                &BRLCAD::Hyperboloid::SemiMajorAxis,
                &BRLCAD::Hyperboloid::SetSemiMajorAxis,
                "Semi Major Axis");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Hyperboloid>(
                &document,
                object,
                &BRLCAD::Hyperboloid::SemiMajorAxisDirection,
                &BRLCAD::Hyperboloid::SetSemiMajorAxisDirection,
                "Semi Major Axis Direction");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Hyperboloid>(
                &document,
                object,
                &BRLCAD::Hyperboloid::SemiMajorAxisLength,
                &BRLCAD::Hyperboloid::SetSemiMajorAxisLength,
                "Semi Major Axis Length");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Hyperboloid>(
                &document,
                object,
                &BRLCAD::Hyperboloid::SemiMinorAxisLength,
                &BRLCAD::Hyperboloid::SetSemiMinorAxisLength,
                "Semi Minor Axis Length");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Hyperboloid>(
                &document,
                object,
                &BRLCAD::Hyperboloid::ApexAsymptoteDistance,
                &BRLCAD::Hyperboloid::SetApexAsymptoteDistance,
                "Apex Asymptote Distance");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "ParabolicCylinder") {
        ObjectDataField<BRLCAD::ParabolicCylinder> * property;

        property = new ObjectDataField<BRLCAD::ParabolicCylinder>(
                &document,
                object,
                &BRLCAD::ParabolicCylinder::BasePoint,
                &BRLCAD::ParabolicCylinder::SetBasePoint,
                "Base Point");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::ParabolicCylinder>(
                &document,
                object,
                &BRLCAD::ParabolicCylinder::Height,
                &BRLCAD::ParabolicCylinder::SetHeight,
                "Height");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::ParabolicCylinder>(
                &document,
                object,
                &BRLCAD::ParabolicCylinder::Depth,
                &BRLCAD::ParabolicCylinder::SetDepth,
                "Depth");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::ParabolicCylinder>(
                &document,
                object,
                &BRLCAD::ParabolicCylinder::HalfWidth,
                &BRLCAD::ParabolicCylinder::SetHalfWidth,
                "Half Width");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "Paraboloid") {
        ObjectDataField<BRLCAD::Paraboloid> * property;

        property = new ObjectDataField<BRLCAD::Paraboloid>(
                &document,
                object,
                &BRLCAD::Paraboloid::BasePoint,
                &BRLCAD::Paraboloid::SetBasePoint,
                "Base Point");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Paraboloid>(
                &document,
                object,
                &BRLCAD::Paraboloid::Height,
                &BRLCAD::Paraboloid::SetHeight,
                "Height");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Paraboloid>(
                &document,
                object,
                &BRLCAD::Paraboloid::SemiMajorAxis,
                &BRLCAD::Paraboloid::SetSemiMajorAxis,
                "Semi Major Axis");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Paraboloid>(
                &document,
                object,
                &BRLCAD::Paraboloid::SemiMajorAxisDirection,
                &BRLCAD::Paraboloid::SetSemiMajorAxisDirection,
                "Semi Major Axis Direction");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Paraboloid>(
                &document,
                object,
                &BRLCAD::Paraboloid::SemiMajorAxisLength,
                &BRLCAD::Paraboloid::SetSemiMajorAxisLength,
                "Semi Major Axis Length");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Paraboloid>(
                &document,
                object,
                &BRLCAD::Paraboloid::SemiMinorAxisLength,
                &BRLCAD::Paraboloid::SetSemiMinorAxisLength,
                "Semi Minor Axis Length");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "Particle") {
        ObjectDataField<BRLCAD::Particle> * property;

        property = new ObjectDataField<BRLCAD::Particle>(
                &document,
                object,
                &BRLCAD::Particle::BasePoint,
                &BRLCAD::Particle::SetBasePoint,
                "Base Point");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Particle>(
                &document,
                object,
                &BRLCAD::Particle::Height,
                &BRLCAD::Particle::SetHeight,
                "Height");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Particle>(
                &document,
                object,
                &BRLCAD::Particle::BaseRadius,
                &BRLCAD::Particle::SetBaseRadius,
                "Base Radius");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Particle>(
                &document,
                object,
                &BRLCAD::Particle::TopRadius,
                &BRLCAD::Particle::SetTopRadius,
                "Top Radius");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "Sphere") {
        ObjectDataField<BRLCAD::Sphere> * property;

        property = new ObjectDataField<BRLCAD::Sphere>(
                &document,
                object,
                &BRLCAD::Sphere::Center,
                &BRLCAD::Sphere::SetCenter,
                "Center");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Sphere>(
                &document,
                object,
                &BRLCAD::Sphere::Radius,
                &BRLCAD::Sphere::SetRadius,
                "Radius");
        l->addWidget(property);
    }


    if(QString(object->Type()) == "Torus") {
        ObjectDataField<BRLCAD::Torus> * property;

        property = new ObjectDataField<BRLCAD::Torus>(
                &document,
                object,
                &BRLCAD::Torus::Center,
                &BRLCAD::Torus::SetCenter,
                "Center");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Torus>(
                &document,
                object,
                &BRLCAD::Torus::Normal,
                &BRLCAD::Torus::SetNormal,
                "Normal");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Torus>(
                &document,
                object,
                &BRLCAD::Torus::TubeCenterLineRadius,
                &BRLCAD::Torus::SetTubeCenterLineRadius,
                "Tube Center Line Radius");
        l->addWidget(property);

        property = new ObjectDataField<BRLCAD::Torus>(
                &document,
                object,
                &BRLCAD::Torus::TubeRadius,
                &BRLCAD::Torus::SetTubeRadius,
                "Tube TubeRadius");
        l->addWidget(property);
    }

    l->addStretch(1);
}

