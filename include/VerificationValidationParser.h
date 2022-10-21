//
// Created by isaacy13 on 10/14/2022.
//

#ifndef VVPARSER_H
#define VVPARSER_H

#include "Utils.h"
#include "VerificationValidation.h"

class VerificationValidationParser {
public:
    static VerificationValidationResult* search(const QString& cmd, const QString* terminalOutput);
    static VerificationValidationResult* lc(const QString* terminalOutput);
    static VerificationValidationResult* gqa(const QString* terminalOutput);
private:
    static void searchSpecificTest(VerificationValidationResult* r, const QString& currentLine, const VerificationValidationTest* type);
    static void searchCatchUsageErrors(VerificationValidationResult* r, const QString& currentLine);
    static void searchDBNotFoundErrors(VerificationValidationResult* r, const QString& currentLine);
    static void searchFinalDefense(VerificationValidationResult* r);
};

#endif