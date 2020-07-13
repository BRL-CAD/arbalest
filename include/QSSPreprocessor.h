
#ifndef RT3_QSSPREPROCESSOR_H
#define RT3_QSSPREPROCESSOR_H


#include <QtCore/qstring.h>

class QSSPreprocessor {
public:
    QSSPreprocessor(QString &theme);
    QString process(QString &styleStr);

private:
    QString theme;
    std::vector<std::pair<QString,QString>> keysValuePairs;
};


#endif //RT3_QSSPREPROCESSOR_H
