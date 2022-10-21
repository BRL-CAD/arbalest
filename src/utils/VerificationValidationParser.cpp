//
// Created by isaacy13 on 10/14/2022.
//

#include "VerificationValidationParser.h"
using DefaultTests = VerificationValidationWidget::DefaultTests;

VerificationValidationResult* VerificationValidationParser::search(const QString& cmd, const QString* terminalOutput) {
    VerificationValidationResult* r = new VerificationValidationResult;
    r->terminalOutput = terminalOutput->trimmed();
    r->resultCode = VerificationValidationResult::Code::PASSED;

    // default checks
    if (QString::compare(DefaultTests::NO_NESTED_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0) {
        
    } 
    
    else if (QString::compare(DefaultTests::NO_EMPTY_COMBOS.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    else if (QString::compare(DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    else if (QString::compare(DefaultTests::ALL_BOTS_VOLUME_MODE.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    else if (QString::compare(DefaultTests::NO_BOTS_LH_ORIENT.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    else if (QString::compare(DefaultTests::ALL_REGIONS_MAT.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    else if (QString::compare(DefaultTests::ALL_REGIONS_LOS.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    else if (QString::compare(DefaultTests::NO_MATRICES.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    else if (QString::compare(DefaultTests::NO_INVALID_AIRCODE_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0) {

    }

    //// general checks
    int msgStart;
    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        QString currentLine = lines[i];

        // catch usage errors
        msgStart = r->terminalOutput.indexOf(QRegExp("usage:", Qt::CaseInsensitive));
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

    // final defense: find any errors / warnings
    if (r->resultCode == VerificationValidationResult::Code::PASSED) {
        msgStart = r->terminalOutput.indexOf(QRegExp("error[: ]", Qt::CaseInsensitive));
        if (msgStart != -1) {
            r->resultCode = VerificationValidationResult::Code::UNPARSEABLE;
            r->issues.push_back({"UNEXPECTED ERROR", r->terminalOutput.mid(msgStart)});
        }

        msgStart = r->terminalOutput.indexOf(QRegExp("warning[: ]", Qt::CaseInsensitive));
        if (msgStart != -1) {
            r->resultCode = VerificationValidationResult::Code::UNPARSEABLE;
            r->issues.push_back({"UNEXPECTED WARNING", r->terminalOutput.mid(msgStart)});
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