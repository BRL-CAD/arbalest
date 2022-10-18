//
// Created by isaacy13 on 10/14/2022.
//

#ifndef VVPARSER_H
#define VVPARSER_H

#include "Utils.h"

class VerificationValidationResult {
public:
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

    QString terminalOutput;
    Code resultCode;
    std::vector<ObjectIssue> issues;
};

class VerificationValidationParser {
public:
    static VerificationValidationResult* search(const QString* terminalOutput);
    static VerificationValidationResult* lc(const QString* terminalOutput);
    static VerificationValidationResult* gqa(const QString* terminalOutput);
};

#endif