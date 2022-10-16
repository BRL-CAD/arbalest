//
// Created by isaacy13 on 10/14/2022.
//

#include "VerificationValidationParser.h"

VerificationValidationResult* VerificationValidationParser::search(const QString& testID, const QString& terminalOutput) {
    VerificationValidationResult* r = new VerificationValidationResult;
    r->testID = testID;
    r->terminalOutput = terminalOutput.trimmed();

    // catch usage errors
    QString errorIdentifier = "usage:";
    int res = r->terminalOutput.indexOf(errorIdentifier, Qt::CaseInsensitive);
    if (res != -1) {
        r->resultCode = VerificationValidationResult::Code::FAILED;
        r->issues.push_back({"SYNTAX ERROR", r->terminalOutput.mid(res)});
        return r;
    }

    // catch not in DB errors
    errorIdentifier = "not found in database";
    res = r->terminalOutput.indexOf(errorIdentifier, Qt::CaseInsensitive);
    if (res != -1) {
        res = r->terminalOutput.indexOf("'", res, Qt::CaseInsensitive) + 1; // find what wasn't found in DB
        r->resultCode = VerificationValidationResult::Code::FAILED;
        r->issues.push_back({, r->terminalOutput.mid(res)});
        return r;
    }

    // final defense: find any errors
    errorIdentifier = "error";
    res = r->terminalOutput.indexOf(errorIdentifier, Qt::CaseInsensitive);
    if (res != -1) {
        r->resultCode = VerificationValidationResult::Code::FAILED;
        r->issues.push_back({"UNEXPECTED ERROR", r->terminalOutput.mid(res)});
        return r;
    }

    r->resultCode = VerificationValidationResult::Code::PASSED;
    return r; // TODO: implement
}

VerificationValidationResult* VerificationValidationParser::lc(const QString& testID, const QString& terminalOutput) {
    return nullptr; // TODO: implement
}
VerificationValidationResult* VerificationValidationParser::gqa(const QString& testID, const QString& terminalOutput) {
    return nullptr; // TODO: implement
}