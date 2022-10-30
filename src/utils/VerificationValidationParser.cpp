//
// Created by isaacy13 on 10/14/2022.
//

#include "Utils.h"
#include "VerificationValidation.h"
#include <iostream> 
#include <sstream> 
#include <cstdlib>

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

    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        // run tests specifically for default tests
        if (type) Parser::searchSpecificTest(r, lines[i], type);

        // catch usage errors
        Parser::searchCatchUsageErrors(r, lines[i]);

        // catch not in DB errors
        Parser::searchDBNotFoundErrors(r, lines[i]);
    }

    // final defense: find any errors / warnings
    if (r->resultCode == Result::Code::PASSED)
        Parser::searchFinalDefense(r);

    return r; // TODO: implement
}

void Parser::searchSpecificTest(Result* r, const QString& currentLine, const Test* type) {
    if (type == &DefaultTests::NO_NESTED_REGIONS) {
        if (currentLine.trimmed().isEmpty()) return;
        r->resultCode = Result::Code::FAILED;
        QStringList objectPath = currentLine.split('/');
        
        // if bad result
        if (objectPath.size() < 2) {
            r->resultCode = Result::Code::UNPARSEABLE;
            r->issues.push_back({"search parser", "failed to parse results"});
            return;
        }

        QString objectName = objectPath.last();
        r->issues.push_back({objectName, "nested region at " + currentLine});
    } 
    
    else if (type == &DefaultTests::NO_EMPTY_COMBOS) {

    }

    else if (type == &DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS) {
        
    }

    else if (type == &DefaultTests::ALL_BOTS_VOLUME_MODE) {
        
    }

    else if (type == &DefaultTests::NO_BOTS_LH_ORIENT) {

    }

    else if (type == &DefaultTests::ALL_REGIONS_MAT) {
        
    }

    else if (type == &DefaultTests::ALL_REGIONS_LOS) {
        
    }

    else if (type == &DefaultTests::NO_MATRICES) {
        
    }

    else if (type == &DefaultTests::NO_INVALID_AIRCODE_REGIONS) {
        
    }
}

void Parser::searchCatchUsageErrors(Result* r, const QString& currentLine) {
    int msgStart = currentLine.indexOf(QRegExp("usage:", Qt::CaseInsensitive));
    if (msgStart != -1) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({"SYNTAX ERROR", currentLine.mid(msgStart)});
    }
}

void Parser::searchDBNotFoundErrors(Result* r, const QString& currentLine) {
    int msgStart = currentLine.indexOf(QRegExp("input: '.*' normalized: '.* not found in database!'", Qt::CaseInsensitive));
    if (msgStart != -1) {
        int objNameStartIdx = msgStart + 8; // skip over "input: '"
        int objNameEndIdx = currentLine.indexOf("'", objNameStartIdx);

        if (objNameStartIdx >= currentLine.size() || objNameEndIdx == -1) {
            r->resultCode = Result::Code::UNPARSEABLE;
            r->issues.push_back({"search parser", "failed to parse results"});
        } else {
            int objNameSz = objNameEndIdx - objNameStartIdx;
            QString objName = currentLine.mid(objNameStartIdx, objNameSz);
            r->resultCode = Result::Code::FAILED;
            r->issues.push_back({objName, currentLine.mid(msgStart)});
        }
    }        
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

Result* Parser::lc(const QString* cmd, const QString* terminalOutput) {
	Result* final = new Result;
	final->terminalOutput = terminalOutput->trimmed();

	/* Check whether it contains any errors or warnings */
	QStringList lines = final->terminalOutput.split("\n");
	if(lines.size() >= 2) {
		final->resultCode = Result::PASSED;
		return final;
	}


	QString issueDescription;
	/* Is it an error or warning? */
	if(cmd->indexOf("-d") != -1) { // This is a Warning
		final->resultCode = Result::WARNING;
		issueDescription = "Contains duplicated ID's";

	}
	else if(cmd->indexOf("-m") != -1) { // This is an Error
		final->resultCode = Result::FAILED; 
		issueDescription = "Contains mismatched ID's";
	}
	else { // If this is neither, assuming it is unparsable
		final->resultCode = Result::UNPARSEABLE;
		return final;
	}

	/* Start adding the issues to list */
	for(size_t i = 0; i < lines.size()-2; i++) {
		/* The +2 is to ignore the list length and list details */
		QStringList temp_parser = lines[i+2].split("  "); // Retrieve data into lists
		Result::ObjectIssue tempObject;
		tempObject.objectName = temp_parser[3].left(temp_parser[3].indexOf('.'));
		tempObject.issueDescription = issueDescription;
		final->issues.push_back(tempObject);
	}
	return final;


}
Result* Parser::gqa(const QString* terminalOutput) {
    return nullptr; // TODO: implement
}
