
#ifndef RT3_OBJECTDATAFIELD_H
#define RT3_OBJECTDATAFIELD_H


#include <include/DataRow.h>
#include <brlcad/Object.h>
#include <brlcad/MemoryDatabase.h>
#include "QVBoxWidget.h"
#include "Document.h"

template<typename T>
class ObjectDataField : public QVBoxWidget {
private:
    Document *document;
public:

    ObjectDataField(
            Document *document,
            void *obj,
            BRLCAD::Vector3D (T::*getter)(size_t) const,
            void (T::*setter)(size_t, const BRLCAD::Vector3D&),
            size_t start,
            size_t count,
            const QStringList & indices,
            QString title
    ) : QVBoxWidget() , document(document) {

        T* object = reinterpret_cast<T*>(obj);
        if (object == nullptr) return;

        if (title != "") {
            if (!indices.isEmpty()) {
                QHBoxWidget * hbox = new QHBoxWidget();
                QLabel *padding = new QLabel(indices[0]);
                QFont font("Monospace");
                font.setStyleHint(QFont::TypeWriter);
                padding->setFont(font);
                padding->setStyleSheet("color:transparent;padding-right:3px;");
                hbox->addWidget(padding);
                QLabel *titleWidget = new QLabel(title);
                titleWidget->setMargin(2);
                hbox->addWidget(titleWidget);
                addWidget(hbox);
                hbox->getBoxLayout()->addStretch();
            }
            else{
                QLabel *titleWidget = new QLabel(title);
                titleWidget->setMargin(2);
                addWidget(titleWidget);
            }
        }

        for (int c=start; c < start+count; c++) {
            BRLCAD::Vector3D val = ((*object).*getter)(c);
            DataRow *row = new DataRow(3, c==start,indices[c-start], this);
            for (int i = 0; i < 3; i++) {
                row->getTextBoxes()[i]->setText(QString::number(val.coordinates[i]));
                if (c!=start+count-1) row->getTextBoxes()[i]->setStyleSheet("border-bottom-width: 0px");

                connect(row->getTextBoxes()[i], &QLineEdit::textEdited, this, [setter,getter,document,object,row,i,c]() {
                    BRLCAD::Vector3D val = ((*object).*getter)(c);
                    val.coordinates[i] = row->getTextBoxes()[i]->text().toDouble();
                    ((*object).*setter)(c,val);
                    document->modifyObject(object);
                });
            }
            addWidget(row);
        }
    }



    ObjectDataField(
            Document *document,
            void *obj,
            BRLCAD::Vector3D (T::*getter)(size_t) const,
            void (T::*setter)(size_t, BRLCAD::Vector3D&),
            size_t start,
            size_t count,
            const QStringList & indices,
            QString title
    ) : QVBoxWidget() , document(document) {

        T* object = reinterpret_cast<T*>(obj);
        if (object == nullptr) return;

        if (title != "") {
            if (!indices.isEmpty()) {
                QHBoxWidget * hbox = new QHBoxWidget();
                QLabel *padding = new QLabel(indices[0]);
                QFont font("Monospace");
                font.setStyleHint(QFont::TypeWriter);
                padding->setFont(font);
                padding->setStyleSheet("color:transparent;padding-right:3px;");
                hbox->addWidget(padding);
                QLabel *titleWidget = new QLabel(title);
                titleWidget->setMargin(2);
                hbox->addWidget(titleWidget);
                addWidget(hbox);
                hbox->getBoxLayout()->addStretch();
            }
            else{
                QLabel *titleWidget = new QLabel(title);
                titleWidget->setMargin(2);
                addWidget(titleWidget);
            }
        }

        for (int c=start; c < start+count; c++) {
            BRLCAD::Vector3D val = ((*object).*getter)(c);
            DataRow *row = new DataRow(3, c==start,indices[c-start], this);
            for (int i = 0; i < 3; i++) {
                row->getTextBoxes()[i]->setText(QString::number(val.coordinates[i]));
                if (c!=start+count-1) row->getTextBoxes()[i]->setStyleSheet("border-bottom-width: 0px");

                connect(row->getTextBoxes()[i], &QLineEdit::textEdited, this, [setter,getter,document,object,row,i,c]() {
                    BRLCAD::Vector3D val = ((*object).*getter)(c);
                    val.coordinates[i] = row->getTextBoxes()[i]->text().toDouble();
                    ((*object).*setter)(c,val);
                    document->modifyObject(object);
                });
            }
            addWidget(row);
        }
    }


    ObjectDataField(
            Document *document,
            void *obj,
            BRLCAD::Vector3D (T::*getter)() const,
            void (T::*setter)(const BRLCAD::Vector3D&),
            QString title
    ) : QVBoxWidget() , document(document) {

        T* object = reinterpret_cast<T*>(obj);
        if (object == nullptr) return;

        if (title != "") {
            QLabel *titleWidget = new QLabel(title);
            titleWidget->setMargin(2);
            addWidget(titleWidget);
        }

        BRLCAD::Vector3D val = ((*object).*getter)();

        DataRow* row = new DataRow(3, true,"", this);
        for (int i = 0; i < 3; i++) {
            row->getTextBoxes()[i]->setText(QString::number(val.coordinates[i]));

            connect(row->getTextBoxes()[i], &QLineEdit::textEdited, this, [setter,getter,document,object,row,i]() {
                BRLCAD::Vector3D val = ((*object).*getter)();
                val.coordinates[i] = row->getTextBoxes()[i]->text().toDouble();
                ((*object).*setter)(val);
                document->modifyObject(object);
            });
        }

        addWidget(row);
    }

    ObjectDataField(
            Document *document,
            void *obj,
            double (T::*getter)() const,
            void (T::*setter)(const double),
            QString title
    ) : QVBoxWidget() , document(document) {

        T* object = reinterpret_cast<T*>(obj);
        if (object == nullptr) return;

        if (title != "") {
            QLabel *titleWidget = new QLabel(title);
            titleWidget->setMargin(2);
            addWidget(titleWidget);
        }

        double val = ((*object).*getter)();

        DataRow* row = new DataRow(1, false,"", this);
        row->getTextBoxes()[0]->setAlignment(Qt::AlignLeft);
        row->getTextBoxes()[0]->setText(QString::number(val));

        connect(row->getTextBoxes()[0], &QLineEdit::textEdited, this, [setter,getter,document,object,row,this]() {
            ((*object).*setter)(row->getTextBoxes()[0]->text().toDouble());
            document->modifyObject(object);
        });

        addWidget(row);
    }
};


#endif //RT3_OBJECTDATAFIELD_H
