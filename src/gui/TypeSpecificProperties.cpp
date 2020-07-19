
#include <QtWidgets/QVBoxLayout>
#include <include/ConeProperties.h>
#include <brlcad/Cone.h>
#include <QtOpenGL/QtOpenGL>
#include "TypeSpecificProperties.h"

#include <QStyledItemDelegate>

class QyAlignmentDelegate : public QStyledItemDelegate
{
public:

    explicit QyAlignmentDelegate(Qt::Alignment alignment, QObject* parent = 0)
            : QStyledItemDelegate(parent),
              m_alignment(alignment)
    {

    }

    virtual void QyAlignmentDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem alignedOption(option);
        alignedOption.displayAlignment = m_alignment;
        QStyledItemDelegate::paint(painter, alignedOption, index);
    }

    QWidget *
    createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        auto editor = QStyledItemDelegate::createEditor(parent, option, index);

        return editor;
    }

private:

    Qt::Alignment   m_alignment;                                                                    ///< Stores the alignment to use

};

TypeSpecificProperties::TypeSpecificProperties(BRLCAD::MemoryDatabase &database, const BRLCAD::Object &object)
        : database(database), object(object) {

    mainVLayout = new QVBoxLayout;
    mainVLayout->setContentsMargins(0,0,0,0);
    setLayout(mainVLayout);
    setObjectName("properties-TypeSpecificProperties");


    int X=3,Y=1;
    QStringList xyzLabels;
    xyzLabels.append("X");
    xyzLabels.append("Y");
    xyzLabels.append("Z");
    QStringList vertLabels;
    vertLabels.append("P1");
    vertLabels.append("Y");
    vertLabels.append("Z");

    QTableWidget * tableWidget = new QTableWidget(Y, X, this);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
    //tableWidget->setStyleSheet("background:transparent;");
    tableWidget->setHorizontalHeaderLabels(xyzLabels);
    tableWidget->setVerticalHeaderLabels(vertLabels);
    this->mainVLayout->addWidget(tableWidget);
    int yo = 25;
    tableWidget->verticalHeader()->setDefaultSectionSize(yo);
    tableWidget->horizontalHeader()->setFixedHeight(20);
    tableWidget->setMaximumHeight(20+Y*yo);
    tableWidget->setMinimumHeight(20+Y*yo);
    auto j = new QTableWidgetItem;
    j->setTextAlignment( Qt::AlignRight);
    j->setBackground(Qt::white);
    tableWidget->setItemPrototype(j);


    tableWidget->setItemDelegate( new QyAlignmentDelegate(Qt::AlignCenter));

    //tableWidget->verticalHeader()->setMaximumWidth(1);

    tableWidget = new QTableWidget(Y, X, this);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
    tableWidget->setStyleSheet("background:transparent;");
    tableWidget->setHorizontalHeaderLabels(xyzLabels);
    tableWidget->setVerticalHeaderLabels(vertLabels);
    this->mainVLayout->addWidget(tableWidget);

    tableWidget = new QTableWidget(Y, X, this);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
    tableWidget->setStyleSheet("background:transparent;");
    tableWidget->setHorizontalHeaderLabels(xyzLabels);
    tableWidget->setVerticalHeaderLabels(vertLabels);
    this->mainVLayout->addWidget(tableWidget);


    mainVLayout->addStretch(1);

}

TypeSpecificProperties *TypeSpecificProperties::build(BRLCAD::MemoryDatabase &database, BRLCAD::Object &object) {
    if (QString(object.Type()) == "Cone"){
        return new ConeProperties(database, dynamic_cast<BRLCAD::Cone&>(object));
    }

    return new TypeSpecificProperties(database, object);
}










//
//for(int y=0;y<Y;y++)for(int x=0;x<X;x++) {
//tableWidget->setItem(y, x, new QTableWidgetItem);
//tableWidget->item(y, x)->setBackground(Qt::white);
//}
