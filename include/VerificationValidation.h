//
// Created by isaacy13 on 10/21/2022.
//

#ifndef VV_H
#define VV_H
#include <vector>
#include <list>
#include <QString>

namespace VerificationValidation {
    class Arg {
    public:
        QString argument;
        bool isVariable;
        QString defaultValue;

        Arg(QString argument, bool isVariable, QString defaultValue){
            this->argument = argument;
            this->isVariable = isVariable;
            this->defaultValue = defaultValue;
        }
    };

    class Test {
    public:
        QString testName;
        QString testCommand;
        QString suiteName;
        QString category;
        bool hasVariable;
        std::vector<Arg> ArgList;
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
        const static VerificationValidation::Test MISMATCHED_DUP_IDS;
        const static VerificationValidation::Test NO_DUPLICATE_ID;
        const static VerificationValidation::Test NO_NULL_REGIONS;
        const static VerificationValidation::Test NO_OVERLAPS;
        const static VerificationValidation::Test NO_NESTED_REGIONS;
        const static VerificationValidation::Test NO_EMPTY_COMBOS;
        const static VerificationValidation::Test NO_SOLIDS_OUTSIDE_REGIONS;
        const static VerificationValidation::Test ALL_BOTS_VOLUME_MODE;
        const static VerificationValidation::Test NO_BOTS_LH_ORIENT; // TODO: this command can run faster if use unix
        const static VerificationValidation::Test ALL_REGIONS_MAT;
        const static VerificationValidation::Test ALL_REGIONS_LOS;
        const static VerificationValidation::Test NO_MATRICES;
        const static VerificationValidation::Test NO_INVALID_AIRCODE_REGIONS;
        const static std::vector<VerificationValidation::Test> allTests;

        // TODO: missing "No errors when top level drawn"
        // TODO: missing "BoTs are valid"
        // TODO: missing "Air does not stick out"
        // TODO: missing "Title"
        // TODO: missing "Ground plane at z=0"
    };

    class Parser {
    public:
        static Result* search(const QString& cmd, const QString* terminalOutput);
        static void searchSpecificTest(Result* r, const QString& currentLine, const Test* type);
        static bool searchCatchUsageErrors(Result* r, const QString& currentLine);
        static bool searchDBNotFoundErrors(Result* r);
        static void searchFinalDefense(Result* r);

        static Result* lc(const QString* terminalOutput);
        static Result* gqa(const QString* terminalOutput);
    };
}

#endif