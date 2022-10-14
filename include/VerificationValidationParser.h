//
// Created by isaacy13 on 10/14/2022.
//

#ifndef VVPARSER_H
#define VVPARSER_H

#include "Utils.h"

class VerificationValidationResult {
    enum Code {
        PASSED,
        WARNING,
        FAILED,
        UNPARSEABLE
    };

    struct ObjectIssue {
        QString objectName;
        QString issueDescription;
    };

    QString testID;
    QString terminalOutput;
    Code resultCode;
    std::vector<ObjectIssue> issues;
};

class VerificationValidationParser {
    static VerificationValidationResult* search(const QString& input);
    static VerificationValidationResult* lc(const QString& input);
    static VerificationValidationResult* gqa(const QString& input);
};

#endif