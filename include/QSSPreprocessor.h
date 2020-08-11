
#ifndef RT3_QSSPREPROCESSOR_H
#define RT3_QSSPREPROCESSOR_H


#include <QtCore/qstring.h>
#include <vector>
#include <QColor>

class QSSPreprocessor {
public:
    QSSPreprocessor(QString &theme);
    QString process(QString styleStr);
    QColor getColor(QString key);

private:
    QString theme;
    std::vector<std::pair<QString,QString>> keysValuePairs;
};


#endif //RT3_QSSPREPROCESSOR_H
