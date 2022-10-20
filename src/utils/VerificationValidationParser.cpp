//
// Created by isaacy13 on 10/14/2022.
//

#include "VerificationValidationParser.h"

VerificationValidationResult* VerificationValidationParser::search(const QString* terminalOutput) {
    VerificationValidationResult* r = new VerificationValidationResult;
    r->terminalOutput = terminalOutput->trimmed();
    r->resultCode = VerificationValidationResult::Code::PASSED;

    int msgStart;
    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        QString currentLine = lines[i];

        // catch usage errors
        msgStart = r->terminalOutput.indexOf(QRegExp("/usage:/g", Qt::CaseInsensitive));
        if (msgStart != -1) {
            r->resultCode = VerificationValidationResult::Code::FAILED;
            r->issues.push_back({"SYNTAX ERROR", currentLine.mid(msgStart)});
        }

        // catch not in DB errors
        msgStart = currentLine.indexOf(QRegExp("input: '.*' normalized: '.* not found in database!'", Qt::CaseInsensitive));
        if (msgStart != -1) {
            int objNameStartIdx = msgStart + 8; // skip over "input: '"
            int objNameEndIdx = currentLine.indexOf("'", objNameStartIdx);

            if (objNameStartIdx >= currentLine.size() || objNameEndIdx == -1) {
                r->resultCode = VerificationValidationResult::Code::UNPARSEABLE;
                r->issues.push_back({"search parser", "failed to parse results"});
            } else {
                int objNameSz = objNameEndIdx - objNameStartIdx;
                QString objName = currentLine.mid(objNameStartIdx, objNameSz);
                r->resultCode = VerificationValidationResult::Code::FAILED;
                r->issues.push_back({objName, currentLine.mid(msgStart)});
            }
        }        
    }

    // final defense: find any errors
    if (r->resultCode == VerificationValidationResult::Code::PASSED) {
        msgStart = r->terminalOutput.indexOf(QRegExp("[eE][rR][rR][oO][rR][: ]", Qt::CaseInsensitive));
        if (msgStart != -1) {
            r->resultCode = VerificationValidationResult::Code::FAILED;
            r->issues.push_back({"UNEXPECTED ERROR", r->terminalOutput.mid(msgStart)});
        }
    }

    return r; // TODO: implement
}

VerificationValidationResult* VerificationValidationParser::lc(const QString* terminalOutput) {
    return nullptr; // TODO: implement
}
VerificationValidationResult* VerificationValidationParser::gqa(const QString* terminalOutput) {
    return nullptr; // TODO: implement
}