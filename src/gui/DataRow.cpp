
#include <include/QVBoxWidget.h>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include "DataRow.h"
#include <QtOpenGL/QtOpenGL>
#include <include/DragEditLineEdit.h>
#include "QHBoxWidget.h"


DataRow::DataRow(int count, bool hasHeader, const QString& indexText, QWidget *parent): QHBoxWidget::QHBoxWidget(parent), count(count), hasHeader(hasHeader) {

    if (indexText != "") {
        QVBoxWidget *vbox = new QVBoxWidget(this);
        if (hasHeader) {
            QLabel *header = new QLabel(vbox);
            header->setMargin(0);
            header->setStyleSheet("background:transparent; border-color:transparent;padding-left:0px;padding-right:3px;");
            vbox->addWidget(header);
        }
        QLabel *textBox = new QLabel(this);
        QFont font("Monospace");
        font.setStyleHint(QFont::TypeWriter);
        textBox->setFont(font);
        textBox->setAlignment(Qt::AlignRight);
        textBox->setText(indexText);
        textBox->setMargin(0);
        textBox->setStyleSheet("background:transparent; border-color:transparent;padding-left:0px;padding-right:3px;");
        vbox->addWidget(textBox);
        this->addWidget(vbox);
    }

    for (int i = 0; i < count; i++) {
        QVBoxWidget *vbox = new QVBoxWidget(this);

        if (hasHeader) {
            QLabel *header = new QLabel(vbox);
            header->setText(headerTexts[i%headerTexts.length()]);
            header->setAlignment(Qt::AlignCenter);
            header->setObjectName("dataRowHeader");
            vbox->addWidget(header);
            headerLabels.push_back(header);
        }

        DragEditLineEdit *textBox = new DragEditLineEdit(this);
        textBox->setObjectName(i == count - 1 ? "cellLastInRow" : "cell");
        textBox->setAlignment(Qt::AlignRight);
        vbox->addWidget(textBox);
        textBoxes.push_back(textBox);

        this->addWidget(vbox);
    }
}

void DataRow::setHeaderTexts(const QStringList &headerTexts) {
    DataRow::headerTexts = headerTexts;
    for (int i = 0; i < count; i++) headerLabels[i]->setText(headerTexts[i%headerTexts.length()]);
}

const std::vector<QLineEdit *> &DataRow::getTextBoxes() const {
    return textBoxes;
}
