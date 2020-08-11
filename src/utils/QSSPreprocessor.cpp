
#include <QString>
#include <QTextStream>
#include <iostream>
#include "QSSPreprocessor.h"

//    *.theme files store the color mapping etc. for a qss file in key value pair format.
//    Syntax highlighting for SCSS files should work with *.theme
//            Syntax example:
//              $primary-color: #333333;
//              $border-color : #ff1111;    // color used for borders
//
//    I could have used a simpler format (ex: "primary-color = #ccc") but this way I can
//            use SCSS settings in the IDE (live color gutter icon etc.)
//
//    QSSPreprocessor class uses the mapping in a .theme file to process a .qss file.
//    It simply replaces "$primary-color" (qss tag must have quotation marks)  with #333333 in the .qss
//
//            Keys or values in .theme cannot contain the following characters and space character
//    ; :
//

QSSPreprocessor::QSSPreprocessor(QString &theme) : theme(theme) {
    QTextStream stream(&theme);

    QString line;
    while (stream.readLineInto(&line)) {
        QString processedLine = line;
        processedLine = processedLine.replace(" ", ""); // no spaces allowed in key or value
        processedLine = processedLine.replace("\r", ""); // just to be sure
        processedLine = processedLine.replace("\n", "");

        if (processedLine.indexOf("//") != -1){
            processedLine = processedLine.left(processedLine.indexOf("//")); // ignore comments like this
        }

        if (processedLine.isEmpty()) continue;

        if(!processedLine.endsWith(";")){
            std::cerr<<"Theme file error. Line not ending with semicolon: "<<line.toStdString()<<std::endl;
            continue;
        }
        processedLine = processedLine.replace(";", "");

        QStringList keyVal = processedLine.split(":");
        if (keyVal.length() != 2){
            std::cerr<<"Theme file error. Cannot parse key value pair: "<<line.toStdString()<<std::endl;
            continue;
        }

        keysValuePairs.push_back(std::pair<QString,QString>(keyVal[0],keyVal[1]));
    }

    // We need to sort by key length DESC to ensure $Color-DockableHeaderText is replaced before $Color-DockableHeader
    auto cmp = [](const std::pair<QString,QString>& a, const std::pair<QString,QString> & b) {
        return a.first.length()>b.first.length();
    };
    std::sort(keysValuePairs.begin(),keysValuePairs.end(),cmp);
}

QString QSSPreprocessor::process(QString styleStr) {
    QString processedStr = styleStr;
    for(std::pair<QString,QString> keyValuePair: keysValuePairs){
        processedStr = processedStr.replace(keyValuePair.first,keyValuePair.second);
    }
    return processedStr;
}

QColor QSSPreprocessor::getColor(const QString key) {
    for(std::pair<QString,QString> keyValuePair: keysValuePairs){
        if(keyValuePair.first == key)
            return QColor(keyValuePair.second);
    }
    return QColor();
}
