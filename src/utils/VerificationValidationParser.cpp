//
// Created by isaacy13 on 10/14/2022.
//

#include "VerificationValidationParser.h"

VerificationValidationResult* VerificationValidationParser::search(const QString& cmd, const QString* terminalOutput) {
    VerificationValidationResult* r = new VerificationValidationResult;
    r->terminalOutput = terminalOutput->trimmed();
    r->resultCode = VerificationValidationResult::Code::PASSED;
    VerificationValidationTest* type = nullptr;

    // default checks
    if (QString::compare(VerificationValidationDefaultTests::NO_NESTED_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::NO_NESTED_REGIONS);
    
    else if (QString::compare(VerificationValidationDefaultTests::NO_EMPTY_COMBOS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::NO_EMPTY_COMBOS);

    else if (QString::compare(VerificationValidationDefaultTests::NO_SOLIDS_OUTSIDE_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::NO_SOLIDS_OUTSIDE_REGIONS);

    else if (QString::compare(VerificationValidationDefaultTests::ALL_BOTS_VOLUME_MODE.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::ALL_BOTS_VOLUME_MODE);

    else if (QString::compare(VerificationValidationDefaultTests::NO_BOTS_LH_ORIENT.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::NO_BOTS_LH_ORIENT);

    else if (QString::compare(VerificationValidationDefaultTests::ALL_REGIONS_MAT.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::ALL_REGIONS_MAT);

    else if (QString::compare(VerificationValidationDefaultTests::ALL_REGIONS_LOS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::ALL_REGIONS_LOS);

    else if (QString::compare(VerificationValidationDefaultTests::NO_MATRICES.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::NO_MATRICES);

    else if (QString::compare(VerificationValidationDefaultTests::NO_INVALID_AIRCODE_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (VerificationValidationTest*) &(VerificationValidationDefaultTests::NO_INVALID_AIRCODE_REGIONS);

    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        // run tests specifically for default tests
        if (type) searchSpecificTest(r, lines[i], type);

        // catch usage errors
        searchCatchUsageErrors(r, lines[i]);

        // catch not in DB errors
        searchDBNotFoundErrors(r, lines[i]);
    }

    // final defense: find any errors / warnings
    if (r->resultCode == VerificationValidationResult::Code::PASSED)
        searchFinalDefense(r);

    return r; // TODO: implement
}

void VerificationValidationParser::searchSpecificTest(VerificationValidationResult* r, const QString& currentLine, const VerificationValidationTest* type) {
    if (type == &VerificationValidationDefaultTests::NO_NESTED_REGIONS) {
        if (currentLine.trimmed().isEmpty()) return;
        r->resultCode = VerificationValidationResult::Code::FAILED;
        QStringList objectPath = currentLine.split('/');
        
        // if bad result
        if (objectPath.size() < 2) {
            r->resultCode = VerificationValidationResult::Code::UNPARSEABLE;
            r->issues.push_back({"search parser", "failed to parse results"});
            return;
        }

        QString objectName = objectPath.last();
        r->issues.push_back({objectName, "nested region at " + currentLine});
    } 
    
    else if (type == &VerificationValidationDefaultTests::NO_EMPTY_COMBOS) {

    }

    else if (type == &VerificationValidationDefaultTests::NO_SOLIDS_OUTSIDE_REGIONS) {
        
    }

    else if (type == &VerificationValidationDefaultTests::ALL_BOTS_VOLUME_MODE) {
        
    }

    else if (type == &VerificationValidationDefaultTests::NO_BOTS_LH_ORIENT) {

    }

    else if (type == &VerificationValidationDefaultTests::ALL_REGIONS_MAT) {
        
    }

    else if (type == &VerificationValidationDefaultTests::ALL_REGIONS_LOS) {
        
    }

    else if (type == &VerificationValidationDefaultTests::NO_MATRICES) {
        
    }

    else if (type == &VerificationValidationDefaultTests::NO_INVALID_AIRCODE_REGIONS) {
        
    }
}

void VerificationValidationParser::searchCatchUsageErrors(VerificationValidationResult* r, const QString& currentLine) {
    int msgStart = currentLine.indexOf(QRegExp("usage:", Qt::CaseInsensitive));
    if (msgStart != -1) {
        r->resultCode = VerificationValidationResult::Code::FAILED;
        r->issues.push_back({"SYNTAX ERROR", currentLine.mid(msgStart)});
    }
}

void VerificationValidationParser::searchDBNotFoundErrors(VerificationValidationResult* r, const QString& currentLine) {
    int msgStart = currentLine.indexOf(QRegExp("input: '.*' normalized: '.* not found in database!'", Qt::CaseInsensitive));
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

void VerificationValidationParser::searchFinalDefense(VerificationValidationResult* r) {
    int msgStart = r->terminalOutput.indexOf(QRegExp("error[: ]", Qt::CaseInsensitive));
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

VerificationValidationResult* VerificationValidationParser::lc(const QString* terminalOutput) {
    return nullptr; // TODO: implement
}
VerificationValidationResult* VerificationValidationParser::gqa(const QString* terminalOutput) {
    return nullptr; // TODO: implement
}