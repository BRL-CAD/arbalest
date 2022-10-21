//
// Created by isaacy13 on 10/14/2022.
//

#ifndef VVPARSER_H
#define VVPARSER_H

#include "Utils.h"
#include "VerificationValidationWidget.h"

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
    std::list<ObjectIssue> issues; // used list for O(1) push_back, O(N) access; since need to display all issues in GUI anyways
};

class VerificationValidationParser {
public:
    static VerificationValidationResult* search(const QString& cmd, const QString* terminalOutput);
    static VerificationValidationResult* lc(const QString* terminalOutput);
    static VerificationValidationResult* gqa(const QString* terminalOutput);
};

#endif