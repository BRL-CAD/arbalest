//
// Created by isaacy13 on 10/14/2022.
//

#include "Utils.h"
#include "VerificationValidation.h"

using Result = VerificationValidation::Result;
using Test = VerificationValidation::Test;
using DefaultTests = VerificationValidation::DefaultTests;
using Parser = VerificationValidation::Parser;

Result* Parser::search(const QString& cmd, const QString* terminalOutput) {
    Result* r = new Result;
    r->terminalOutput = terminalOutput->trimmed();
    r->resultCode = Result::Code::PASSED;
    Test* type = nullptr;

    // default checks
    if (QString::compare(DefaultTests::NO_NESTED_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_NESTED_REGIONS);
    
    else if (QString::compare(DefaultTests::NO_EMPTY_COMBOS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_EMPTY_COMBOS);

    else if (QString::compare(DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS);

    else if (QString::compare(DefaultTests::ALL_BOTS_VOLUME_MODE.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::ALL_BOTS_VOLUME_MODE);

    else if (QString::compare(DefaultTests::NO_BOTS_LH_ORIENT.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_BOTS_LH_ORIENT);

    else if (QString::compare(DefaultTests::ALL_REGIONS_MAT.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::ALL_REGIONS_MAT);

    else if (QString::compare(DefaultTests::ALL_REGIONS_LOS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::ALL_REGIONS_LOS);

    else if (QString::compare(DefaultTests::NO_MATRICES.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_MATRICES);

    else if (QString::compare(DefaultTests::NO_INVALID_AIRCODE_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_INVALID_AIRCODE_REGIONS);

    // search for DB errors (if found, return)
    if (Parser::searchDBNotFoundErrors(r)) return r;
    
    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        // if no usage errors, run specific test
        if (!Parser::searchCatchUsageErrors(r, lines[i]) && type)
            Parser::searchSpecificTest(r, lines[i], type);
    }

    // final defense: find any errors / warnings
    if (r->resultCode == Result::Code::PASSED)
        Parser::searchFinalDefense(r);

    return r;
}

void Parser::searchSpecificTest(Result* r, const QString& currentLine, const Test* type) {
    if (currentLine.trimmed().isEmpty()) return;
    QString objectPath = currentLine;
    QString objectName = currentLine.split('/').last();

    if (type == &DefaultTests::NO_NESTED_REGIONS) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objectPath, "Nested region at '" + objectName + "'"});
    } 
    
    else if (type == &DefaultTests::NO_EMPTY_COMBOS) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objectPath, "Empty combo at '" + objectName + "'"});
    }

    else if (type == &DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objectPath, "Solid outside of region at '" + objectName + "'"});
    }

    else if (type == &DefaultTests::ALL_BOTS_VOLUME_MODE) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objectPath, "BoT not volume mode at '" + objectName + "'"});
    }

    else if (type == &DefaultTests::NO_BOTS_LH_ORIENT) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objectPath, "Left-hand oriented BoT at '" + objectName + "'"});
    }

    else if (type == &DefaultTests::ALL_REGIONS_MAT) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objectPath, "Obj/region doesn't have material at '" + objectName + "'"});
    }

    else if (type == &DefaultTests::ALL_REGIONS_LOS) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objectPath, "Obj/region doesn't have LOS at '" + objectName + "'"});
    }

    else if (type == &DefaultTests::NO_MATRICES) {
        r->resultCode = Result::Code::WARNING;
        r->issues.push_back({objectPath, "Matrix at '" + objectName + "'"});
    }

    else if (type == &DefaultTests::NO_INVALID_AIRCODE_REGIONS) {
        r->resultCode = Result::Code::WARNING;
        r->issues.push_back({objectPath, "Obj/region has aircode at '" + objectName + "'"});
    }
}

bool Parser::searchCatchUsageErrors(Result* r, const QString& currentLine) {
    int msgStart = currentLine.indexOf(QRegExp("usage:", Qt::CaseInsensitive));
    if (msgStart != -1) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({"SYNTAX ERROR", currentLine.mid(msgStart)});
        return true;
    }
    return false;
}

bool Parser::searchDBNotFoundErrors(Result* r) {
    int msgStart = r->terminalOutput.indexOf(QRegExp("Search path error:\n input: '.*' normalized: '.* not found in database!'", Qt::CaseInsensitive));
    if (msgStart != -1) {
        int objNameStartIdx = msgStart + 28; // skip over "Search path error:\n input: '"
        int objNameEndIdx = r->terminalOutput.indexOf("'", objNameStartIdx);
        
        int objNameSz = objNameEndIdx - objNameStartIdx;
        QString objName = r->terminalOutput.mid(objNameStartIdx, objNameSz);
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({objName, r->terminalOutput.mid(msgStart)});

        return true;
    }
    return false; 
}

void Parser::searchFinalDefense(Result* r) {
    int msgStart = r->terminalOutput.indexOf(QRegExp("error[: ]", Qt::CaseInsensitive));
    if (msgStart != -1) {
        r->resultCode = Result::Code::UNPARSEABLE;
        r->issues.push_back({"UNEXPECTED ERROR", r->terminalOutput.mid(msgStart)});
    }

    msgStart = r->terminalOutput.indexOf(QRegExp("warning[: ]", Qt::CaseInsensitive));
    if (msgStart != -1) {
        r->resultCode = Result::Code::UNPARSEABLE;
        r->issues.push_back({"UNEXPECTED WARNING", r->terminalOutput.mid(msgStart)});
    }
}

Result* Parser::lc(const QString* terminalOutput) {
    return nullptr; // TODO: implement
}

Result* Parser::gqa(const QString& cmd, const QString* terminalOutput) {
    Result* r = new Result;
    r->terminalOutput = terminalOutput->trimmed();
    r->resultCode = Result::Code::PASSED;
    Test* type = nullptr;

    if (QString::compare(DefaultTests::NO_NULL_REGIONS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_NULL_REGIONS);
    
    else if (QString::compare(DefaultTests::NO_OVERLAPS.testCommand, cmd, Qt::CaseInsensitive) == 0)
        type = (Test*) &(DefaultTests::NO_OVERLAPS);

    // search for DB errors (if found, return)
    if (Parser::gqaDBNotFoundErrors(r)) return r;
    
    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        // if no usage errors, run specific test
        if (!Parser::gqaCatchUsageErrors(r, lines[i]) && type)
            Parser::gqaSpecificTest(r, lines[i], type);
    }

    // final defense: find any errors / warnings
    if (r->resultCode == Result::Code::PASSED)
        Parser::gqaFinalDefense(r);

    return r;
}

void Parser::gqaSpecificTest(Result* r, const QString& currentLine, const Test* type) {
    if (currentLine.trimmed().startsWith("Trying initial grid spacing", Qt::CaseInsensitive) == true || 
        currentLine.trimmed().startsWith("Using grid spacing lower limit", Qt::CaseInsensitive) == true ||
        currentLine.trimmed().startsWith("Processing with grid spacing", Qt::CaseInsensitive) == true ||
        currentLine.trimmed().startsWith("overlap tolerance to", Qt::CaseInsensitive) == true ||
        currentLine.trimmed().startsWith("NOTE: Stopped, grid spacing refined to", Qt::CaseInsensitive) == true ||
        currentLine.trimmed().startsWith("Trying initial grid spacing", Qt::CaseInsensitive) == true ||
        currentLine.trimmed().startsWith("Summary", Qt::CaseInsensitive) == true ||
        currentLine.trimmed().startsWith("list Overlaps", Qt::CaseInsensitive) == true ||
        currentLine.trimmed().isEmpty()) return;
    
    if(type == &DefaultTests::NO_NULL_REGIONS)
    {
        QString objectPath1 = currentLine.split(' ')[0];
        QString objectName1 = objectPath1.mid(objectPath1.lastIndexOf('/'), objectPath1.size() - objectPath1.lastIndexOf('/'));

        if(currentLine.contains("was not hit"))
        {
            r->resultCode = Result::Code::FAILED;
            r->issues.push_back({objectName1, "Was not hit"});
        }
    }

    if(type == &DefaultTests::NO_OVERLAPS)
    {
        if(currentLine.contains("was not hit")) return;

        r->resultCode = Result::Code::WARNING;
        // [NOTE] Have not checked this yet, assuming similar parse
        QString objectPath1 = currentLine.split(' ')[0];
        QString objectName1 = objectPath1.mid(objectPath1.lastIndexOf('/'), objectPath1.size() - objectPath1.lastIndexOf('/'));
        QString objectPath2 = currentLine.split(' ')[1];
        QString objectName2 = objectPath1.mid(objectPath2.lastIndexOf('/'), objectPath2.size() - objectPath2.lastIndexOf('/'));
        QString countString = currentLine.split(' ')[2];
        QString distanceString = currentLine.split(' ')[3];
        QString locationString = currentLine.mid(currentLine.indexOf('('), currentLine.size() - currentLine.indexOf('('));

        r->issues.push_back({objectName1, "Overlaps with '" + objectName2 + "' " + countString});
    }
    
}


bool Parser::gqaCatchUsageErrors(Result* r, const QString& currentLine) {
    return false;
}

bool Parser::gqaDBNotFoundErrors(Result* r) {
    return false;
}

void Parser::gqaFinalDefense(Result* r) {
    
}