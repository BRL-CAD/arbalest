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
	Result final;
	final.terminalOutput = *terminalOutput;
	final.cmd = *cmd;
	Result* ptr_final = &final;

	/* Convert terminal output and cmd into string for ease of parsing */
	string s_terminalOutput = terminalOutput->toStdString();
	string s_cmd = cmd->toStdString();

	/* Check if length of list is zero, aka does it warnings or errors*/
	if(stoi(s_terminalOutput.substr(s_terminalOutput.find(":") + 1)) == 0) { // Contains no issues
		final.resultCode = Result::PASSED;
		return ptr_final;
	}


	string issueDescription;
	/* Is it an error or warning? */
	if(s_cmd.find("-d") != string::npos) { // This is a Warning
		final.resultCode = Result::WARNING;
		issueDescription = "Contains duplicated ID's";

	}
	else if(s_cmd.find("-m") != string::npos) { // This is an Error
		final.resultCode = Result::FAILED; 
		issueDescription = "Contains mismatched ID's";
	}
	else { // If this is neither, assuming it is unparsable
		final.resultCode = Result::UNPARSEABLE;
		return ptr_final;
	}

	/* Start adding the issues to list */
	std::list<Result::ObjectIssue> temp_issues; // List for issues
	string temp; // Variable used for storing line of entry
	istringstream iss(s_terminalOutput);
	getline(iss, temp); // Gets rid of length line
	getline(iss, temp); // Gets rid of column description
	int count;	
	/* Parse through the results */ 
	for(int i = 0; i < stoi(s_terminalOutput.substr(s_terminalOutput.find(":")+1)); i++) {
		getline(iss, temp);
		istringstream ess(temp);
		count = 5; // This is where the name of object is stored
		while(count-- > 0 && (ess >> temp)); // Get to the name of object
		Result::ObjectIssue tempObject;
		tempObject.objectName = QString::fromStdString(temp.substr(0, temp.find(".")));
		tempObject.issueDescription = QString::fromStdString(issueDescription);
		temp_issues.push_back(tempObject);
	}
	final.issues = temp_issues;
	return ptr_final;


}
Result* Parser::gqa(const QString* terminalOutput) {
    return nullptr; // TODO: implement
}
