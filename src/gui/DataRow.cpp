
#include <include/QVBoxWidget.h>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include "DataRow.h"
#include <QtOpenGL/QtOpenGL>
#include "QHBoxWidget.h"


static void setLineEditTextFormat(QLineEdit *lineEdit, const QList<QTextLayout::FormatRange> &formats) {
    if (!lineEdit) return;
    QList<QInputMethodEvent::Attribute> attributes;
    foreach(const QTextLayout::FormatRange &fr, formats) {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = fr.start - lineEdit->cursorPosition();
        int length = fr.length;
        QVariant value = fr.format;
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
    }
    QInputMethodEvent event(QString(), attributes);
    QCoreApplication::sendEvent(lineEdit, &event);
}
static void clearLineEditTextFormat(QLineEdit *lineEdit) {
    setLineEditTextFormat(lineEdit, QList<QTextLayout::FormatRange>());
}

DataRow::DataRow(int count, bool hasHeader, QWidget *parent): QHBoxWidget::QHBoxWidget(parent), count(count), hasHeader(hasHeader) {
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

        QLineEdit *textBox = new QLineEdit(this);
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
