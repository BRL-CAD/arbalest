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

bool Parser::catchUsageErrors(Result* r, const QString& currentLine) {
    int msgStart = currentLine.indexOf(QRegExp("usage:", Qt::CaseInsensitive));
    if (msgStart != -1) {
        r->resultCode = Result::Code::FAILED;
        r->issues.push_back({"SYNTAX ERROR", currentLine.mid(msgStart)});
        return true;
    }
    return false;
}

void Parser::finalDefense(Result* r) {
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

Result* Parser::search(const QString& cmd, const QString& terminalOutput, const Test& test) {
    Result* r = new Result;
    r->terminalOutput = terminalOutput.trimmed();
    r->resultCode = Result::Code::PASSED;
    Test* type = nullptr;

    // default checks
    if (DefaultTests::NO_NESTED_REGIONS.isSameType(test))
        type = (Test*) &(DefaultTests::NO_NESTED_REGIONS);
    
    else if (DefaultTests::NO_EMPTY_COMBOS.isSameType(test))
        type = (Test*) &(DefaultTests::NO_EMPTY_COMBOS);

    else if (DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS.isSameType(test))
        type = (Test*) &(DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS);

    else if (DefaultTests::ALL_BOTS_VOLUME_MODE.isSameType(test))
        type = (Test*) &(DefaultTests::ALL_BOTS_VOLUME_MODE);

    else if (DefaultTests::NO_BOTS_LH_ORIENT.isSameType(test))
        type = (Test*) &(DefaultTests::NO_BOTS_LH_ORIENT);

    else if (DefaultTests::ALL_REGIONS_MAT.isSameType(test))
        type = (Test*) &(DefaultTests::ALL_REGIONS_MAT);

    else if (DefaultTests::ALL_REGIONS_LOS.isSameType(test))
        type = (Test*) &(DefaultTests::ALL_REGIONS_LOS);

    else if (DefaultTests::NO_MATRICES.isSameType(test))
        type = (Test*) &(DefaultTests::NO_MATRICES);

    else if (DefaultTests::NO_INVALID_AIRCODE_REGIONS.isSameType(test))
        type = (Test*) &(DefaultTests::NO_INVALID_AIRCODE_REGIONS);

    // search for DB errors (if found, return)
    if (Parser::searchDBNotFoundErrors(r)) return r;
    
    QStringList lines = r->terminalOutput.split('\n');
    for (size_t i = 0; i < lines.size(); i++) {
        // if no usage errors, run specific test
        if (!Parser::catchUsageErrors(r, lines[i]) && type)
            Parser::searchSpecificTest(r, lines[i], type);
    }

    // final defense: find any errors / warnings
    if (r->resultCode == Result::Code::PASSED)
        Parser::finalDefense(r);

    return r;
}

void Parser::searchSpecificTest(Result* r, const QString& currentLine, const Test* type) {
    if (currentLine.trimmed().isEmpty()) return;
    QString objectPath = currentLine;
    QString objectName = currentLine.split('/').last().trimmed();

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

Result* Parser::title(const QString& cmd, const QString& terminalOutput, const Test& test) {
    Result* r = new Result;
    r->terminalOutput = terminalOutput.trimmed();
    r->resultCode = Result::Code::PASSED;

    QStringList cmdList = cmd.split(" ", Qt::SkipEmptyParts);
    if (cmdList.size() > 1) {
        r->resultCode = Result::Code::UNPARSEABLE;
        r->issues.push_back({"SYNTAX ERROR", "title cannot have any arguments for testing (implies setting database name)"});
    }

    bool suspiciousTitle = terminalOutput.contains("tmp", Qt::CaseInsensitive) || 
    terminalOutput.contains("temporary", Qt::CaseInsensitive) || 
    terminalOutput.contains("untitled", Qt::CaseInsensitive);
    
    if (suspiciousTitle) {
        r->resultCode = Result::Code::WARNING;
        r->issues.push_back({terminalOutput, "title contains a keyword indicating it is not finalized"});
    }

    return r;
}

Result* Parser::lc(const QString& cmd, const QString& terminalOutput, const QString& gFilePath) {
	Result* r = new Result;
	r->terminalOutput = terminalOutput.trimmed();
	
	/* Check if database exists */
	if(r->terminalOutput.contains("does not exist.", Qt::CaseInsensitive)) {
		r->resultCode = Result::Code::FAILED;
        r->issues.push_back({"Database doesn't exist", terminalOutput});
		return r;
	}

    if(r->terminalOutput.contains("More than one group name was specified", Qt::CaseInsensitive)) {
		r->resultCode = Result::Code::FAILED;
        r->issues.push_back({"Database doesn't exist", terminalOutput});
		return r;
	}

	/* Check if its just usage */
	if(QString::compare(cmd.trimmed(), "lc", Qt::CaseInsensitive) == 0) {
		r->resultCode = Result::Code::FAILED;
        r->issues.push_back({"SYNTAX ERROR", terminalOutput});
		return r;
	}

	QStringList lines = r->terminalOutput.split("\n");
	/* Retreieve the list length */
	QStringList number = lines[0].split(" ");
	int list_length = 0;
	QRegExp re("\\d");
	for(int i = 0; i < number.size(); i++) {
		if(re.exactMatch(number[i])) {
			if(number[i].toInt() == 0) {
				r->resultCode = Result::Code::PASSED;
				return r;
			}
			else {
				list_length = number[i].toInt();
			}
		}
	}

	int dFlagIdx = cmd.indexOf("-d");
    int mFlagIdx = cmd.indexOf("-m");
    /* Is it an error or warning? */
    if (dFlagIdx != -1 && mFlagIdx != -1) {
        r->resultCode = Result::Code::UNPARSEABLE;
        r->issues.push_back({"Unknown combination of flags in command", cmd});
		return r;
    }
	else if(dFlagIdx != -1) { // This is a Warning
		r->resultCode = Result::Code::WARNING;
	}
	else if(mFlagIdx != -1) { // This is an Error
		r->resultCode = Result::Code::FAILED; 
	}
	else { // If this is neither, assuming it is unparsable
		r->resultCode = Result::Code::UNPARSEABLE;
        r->issues.push_back({"Unknown flag in command", cmd});
		return r;
	}

    QString object = cmd.trimmed().split(" ").last();
    /* Start adding the issues to list */
	for(size_t i = 0; i < list_length; i++) {
		/* The +2 is to ignore the list length and list details */
		QStringList temp_parser = lines[i+2].split(QRegExp("\\s+"), Qt::SkipEmptyParts); // Retrieve data into lists
        if (temp_parser.size() != 6) {
            r->resultCode = Result::Code::UNPARSEABLE;
            r->issues.push_back({"Cannot parse unexpected output (was expecting 6 columns)", "was expecting 6 columns, found " + QString::number(temp_parser.size())});
            return r;
        }

        QString ID = temp_parser[0];
        QString MAT = temp_parser[1];
        QString LOS = temp_parser[2];
        QString AIR = temp_parser[3];
        QString REGION = temp_parser[4];
        QString PARENT = temp_parser[5];

		QString issueDescription = "(ID,MAT,LOS,AIR,REGION,PARENT) = ("+ID+","+MAT+","+LOS+","+AIR+","+REGION+","+PARENT+")";
        QString objectPath = "COULD NOT OBTAIN FULL PATH";
        QString searchCMD = "search / -path /%1/\\*%2/%3";
        const QString result = mgedRun(searchCMD.arg(object).arg(PARENT).arg(REGION), gFilePath).trimmed();
        if (!result.isEmpty()) objectPath = result;
        r->issues.push_back({objectPath, issueDescription});
	}

	return r;
}

Result* Parser::gqa(const QString& cmd, const QString& terminalOutput, const Test& test) {
    Result* r = new Result;
    r->terminalOutput = terminalOutput.trimmed();
    r->resultCode = Result::Code::PASSED;
    Test* type = nullptr;

    if (DefaultTests::NO_NULL_REGIONS.isSameType(test) && test.testName == DefaultTests::NO_NULL_REGIONS.testName)
        type = (Test*) &(DefaultTests::NO_NULL_REGIONS);
    
    else if (DefaultTests::NO_OVERLAPS.isSameType(test) && test.testName == DefaultTests::NO_OVERLAPS.testName)
        type = (Test*) &(DefaultTests::NO_OVERLAPS);
    
    QStringList lines = r->terminalOutput.split('\n');
    bool startParsing = false;
    for (size_t i = 0; i < lines.size(); i++) {
        // if no usage errors, run specific test
        if(lines[i].startsWith("list Overlaps"))
        {
            startParsing = true;
            continue;
        }
        if (!Parser::catchUsageErrors(r, lines[i]) && type && startParsing)
            Parser::gqaSpecificTest(r, lines[i], type);
    }

    // final defense: find any errors / warnings
    if (r->resultCode == Result::Code::PASSED)
        Parser::finalDefense(r);

    return r;
}

void Parser::gqaSpecificTest(Result* r, const QString& currentLine, const Test* type) {
    if (currentLine.trimmed().isEmpty()) return;
    
    if(type == &DefaultTests::NO_NULL_REGIONS)
    {
        QString objectPath1 = currentLine.split(' ')[0];
        QString objectName1 = objectPath1;
        int slashIdx1 = objectPath1.lastIndexOf('/');
        if(slashIdx1 != -1)
            objectName1 = objectPath1.mid(slashIdx1 + 1, objectPath1.size() - slashIdx1 - 1);

        if(currentLine.contains("was not hit"))
        {
            r->resultCode = Result::Code::FAILED;
            r->issues.push_back({objectPath1, "'"+objectName1+"' was not hit"});
        }
    }

    else if(type == &DefaultTests::NO_OVERLAPS)
    {
        if(currentLine.contains("was not hit")) return;

        r->resultCode = Result::Code::WARNING;
        QStringList splitLine = currentLine.split(' ');

        QString objectPath1 = splitLine[0];
        QString objectName1 = objectPath1;
        int slashIdx1 = objectPath1.lastIndexOf('/');
        if(slashIdx1 != -1)
            objectName1 = objectPath1.mid(slashIdx1 + 1, objectPath1.size() - slashIdx1 - 1);

        QString objectPath2 = splitLine[1];
        QString objectName2 = objectPath2;
        int slashIdx2 = objectPath2.lastIndexOf('/');
        if(slashIdx2 != -1)
            objectName2 = objectPath2.mid(slashIdx2 + 1, objectPath2.size() - slashIdx2 - 1);

        QString countString = splitLine[2].mid(6, splitLine[2].size() - 6);
        QString distanceString = splitLine[3].mid(5, splitLine[3].size() - 5);
        int parenthIdx = currentLine.indexOf('(');
        QString locationString = currentLine.mid(parenthIdx + 1, currentLine.size() - parenthIdx - 1);

        r->issues.push_back({objectPath1, "'"+objectName1+"' overlaps with '" + objectName2 + "' -- Count: " + countString + ", Distance: "+distanceString});
    }
    
}
