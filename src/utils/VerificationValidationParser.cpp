//
// Created by isaacy13 on 10/14/2022.
//

#include "VerificationValidationParser.h"

VerificationValidationResult* VerificationValidationParser::search(const QString& testID, const QString& terminalOutput) {
    VerificationValidationResult* r = new VerificationValidationResult;
    r->testID = testID;
    r->terminalOutput = terminalOutput.trimmed();
    r->resultCode = VerificationValidationResult::Code::PASSED;

    QString errorIdentifier;
    int res;
    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        QString currentLine = lines[i];

        // catch usage errors
        errorIdentifier = "usage:";
        res = currentLine.indexOf(errorIdentifier, Qt::CaseInsensitive);
        if (res != -1) {
            r->resultCode = VerificationValidationResult::Code::FAILED;
            r->issues.push_back({"SYNTAX ERROR", currentLine.mid(res)});
        }

        // catch not in DB errors
        errorIdentifier = "not found in database";
        res = currentLine.indexOf(errorIdentifier, Qt::CaseInsensitive);
        if (res != -1) {
            // TODO: this is currently incorrect; rfind this, not find
            int msgStart = currentLine.indexOf("'", res, Qt::CaseInsensitive) + 1; // find what wasn't found in DB
            r->resultCode = VerificationValidationResult::Code::FAILED;
            r->issues.push_back({"TODO: put object name here", currentLine.mid(msgStart)});
        }        
    }

    // final defense: find any errors
    if (r->issues.size() == 0) {
        errorIdentifier = "error";
        res = r->terminalOutput.indexOf(errorIdentifier, Qt::CaseInsensitive);
        if (res != -1) {
            r->resultCode = VerificationValidationResult::Code::FAILED;
            r->issues.push_back({"UNEXPECTED ERROR", r->terminalOutput.mid(res)});
        }
    }

    return r; // TODO: implement
}

VerificationValidationResult* VerificationValidationParser::lc(const QString& testID, const QString& terminalOutput) {
    return nullptr; // TODO: implement
}
VerificationValidationResult* VerificationValidationParser::gqa(const QString& testID, const QString& terminalOutput) {
    return nullptr; // TODO: implement
}