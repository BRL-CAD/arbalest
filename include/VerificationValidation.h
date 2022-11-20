//
// Created by isaacy13 on 10/21/2022.
//

#ifndef VV_H
#define VV_H
#include <vector>
#include <list>
#include <map>
#include <QString>
#include "ObjectTreeWidget.h"

namespace VerificationValidation {
    class Arg {
    public:
        enum Type {
            Static, // argument is not variable (e.g.: "-t")
            Dynamic, // argument is variable (e.g.: "-t3mm,3mm" or "-t5mm,5mm")
            ObjectNone, // only used to determine when a command has no object
            ObjectName, // argument is the objectName
            ObjectPath // argument is the objectPath
        };

        QString argument;
        int argIdx;
        QString defaultValue;
        Type type;

        Arg(QString argument, QString defaultValue = NULL, Type type = Static){
            this->argument = argument;
            this->defaultValue = defaultValue;

            if (defaultValue != NULL) this->type = Dynamic;
            else this->type = type;
        }

        bool operator<(const Arg& rhs) { return argument < rhs.argument; }
    };

    class Test {
    public:
        QString testName;
        QString testCommand;
        QStringList suiteNames;
        QString category;
        std::vector<Arg> ArgList;

        Test(const QString& testName, const QStringList& suiteNames, const std::vector<Arg>& ArgList, const QString category = NULL) :
        testName(testName), suiteNames(suiteNames), ArgList(ArgList)
        {
            if (!ArgList.size()) throw std::runtime_error("ArgList must be populated for a test");
            else this->testCommand = ArgList[0].argument;
            
            if (category != NULL) this->category = category;
            else if (ArgList.size()) this->category = ArgList[0].argument;
            else this->category = "NULL";

            for (int i = 0; i < this->ArgList.size(); i++)
                this->ArgList[i].argIdx = i;
        }

        bool isSameType(const Test& rhs) {
            if (ArgList.size() != rhs.ArgList.size()) return false;

            std::vector<Arg> lhsArgList(ArgList);
            std::vector<Arg> rhsArgList(rhs.ArgList);
            std::sort(lhsArgList.begin(), lhsArgList.end());
            std::sort(rhsArgList.begin(), rhsArgList.end());

            for (int i = 0; i < lhsArgList.size(); i++) {
                if (lhsArgList[i].type == rhsArgList[i].type && (lhsArgList[i].type == Arg::Type::ObjectName || lhsArgList[i].type == Arg::Type::ObjectNone || lhsArgList[i].type == Arg::Type::ObjectPath))
                    continue;
                if (lhsArgList[i].argument != rhsArgList[i].argument)
                    return false;
            }
            return true;
        }

        bool operator==(const Test& rhs) {
            if (ArgList.size() != rhs.ArgList.size()) return false;

            std::vector<Arg> lhsArgList(ArgList);
            std::vector<Arg> rhsArgList(rhs.ArgList);
            std::sort(lhsArgList.begin(), lhsArgList.end());
            std::sort(rhsArgList.begin(), rhsArgList.end());

            for (int i = 0; i < lhsArgList.size(); i++) {
                if (lhsArgList[i].type == rhsArgList[i].type && (lhsArgList[i].type == Arg::Type::ObjectName || lhsArgList[i].type == Arg::Type::ObjectNone || lhsArgList[i].type == Arg::Type::ObjectPath))
                    continue;
                if (lhsArgList[i].argument != rhsArgList[i].argument || lhsArgList[i].defaultValue != rhsArgList[i].defaultValue)
                    return false;
            }
            return true;
        }

        bool operator!=(const Test& rhs) {
            return !operator==(rhs);
        }

        bool hasVarArgs() const {
            for (int i = 0; i < ArgList.size(); i++)
                if (ArgList[i].type == Arg::Type::Dynamic) return true;
            return false;
        }

        Arg::Type getObjArgType() const {
            for (int i = 0; i < ArgList.size(); i++) {
                if (ArgList[i].type == Arg::Type::ObjectName || ArgList[i].type == Arg::Type::ObjectPath) 
                    return ArgList[i].type;
            }
            return Arg::Type::ObjectNone;
        }

        QString getCMD(const QString& object = NULL) const {
            QString cmd = "";
            bool addedObject = false;
            for(int i = 0; i < ArgList.size(); i++){
                QString arg = ArgList[i].argument;
                if ((ArgList[i].type == Arg::Type::ObjectName || ArgList[i].type == Arg::Type::ObjectPath) && object != NULL) {
                    if (!addedObject) {
                        cmd += object;
                        if (i + 1 != ArgList.size() && !object.isEmpty()) cmd += " ";
                        addedObject = true;
                    }
                }

                else  {
                    cmd += arg;
                    if (i + 1 != ArgList.size() && !arg.isEmpty()) cmd += " ";
                }
                
                if (ArgList[i].type == Arg::Type::Dynamic) {
                    cmd += ArgList[i].defaultValue;
                    if (i + 1 != ArgList.size() && !ArgList[i].defaultValue.isEmpty()) cmd += " ";
                }
            }
            return cmd;
        }
    };

    class Result {
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

    class DefaultTests {
    public:
        static VerificationValidation::Test MISMATCHED_DUP_IDS;
        static VerificationValidation::Test NO_DUPLICATE_ID;
        static VerificationValidation::Test NO_NULL_REGIONS;
        static VerificationValidation::Test NO_OVERLAPS;
        static VerificationValidation::Test NO_NESTED_REGIONS;
        static VerificationValidation::Test NO_EMPTY_COMBOS;
        static VerificationValidation::Test NO_SOLIDS_OUTSIDE_REGIONS;
        static VerificationValidation::Test ALL_BOTS_VOLUME_MODE;
        static VerificationValidation::Test NO_BOTS_LH_ORIENT; // TODO: this command can run faster if use unix
        static VerificationValidation::Test ALL_REGIONS_MAT;
        static VerificationValidation::Test ALL_REGIONS_LOS;
        static VerificationValidation::Test NO_MATRICES;
        static VerificationValidation::Test NO_INVALID_AIRCODE_REGIONS;
        static VerificationValidation::Test VALID_TITLE;
        const static std::vector<VerificationValidation::Test*> allTests;
        const static std::map<QString, VerificationValidation::Test> nameToTestMap;

        // TODO: missing "No errors when top level drawn"
        // TODO: missing "BoTs are valid"
        // TODO: missing "Air does not stick out"
        // TODO: missing "Ground plane at z=0"
    };

    class Parser {
    public:
        static Result* search(const QString& cmd, const QString& terminalOutput, const Test& test);
        static void searchSpecificTest(Result* r, const QString& currentLine, const Test* type);
        static bool searchCatchUsageErrors(Result* r, const QString& currentLine);
        static bool searchDBNotFoundErrors(Result* r);

        static Result* title(const QString& cmd, const QString& terminalOutput, const Test& test);
        static Result* lc(const QString& cmd, const QString& terminalOutput, const QString& gFilePath);
        static Result* gqa(const QString& cmd, const QString& terminalOutput, const Test& test);
        static void gqaSpecificTest(Result* r, const QString& currentLine, const Test* type);

        static bool catchUsageErrors(Result* r, const QString& currentLine);
        static void finalDefense(Result* r);
    };
}

#endif
