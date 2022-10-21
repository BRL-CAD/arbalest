//
// Created by isaacy13 on 10/21/2022.
//

#ifndef VVTESTS_H
#define VVTESTS_H
#include <vector>
#include <list>
#include <QString>

class VerificationValidationTest {
public:
    QString testName;
    QString testCommand;
    QString suiteName;
};

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

class VerificationValidationDefaultTests {
public:
    const static VerificationValidationTest MISMATCHED_DUP_IDS;
    const static VerificationValidationTest NO_NESTED_REGIONS;
    const static VerificationValidationTest NO_EMPTY_COMBOS;
    const static VerificationValidationTest NO_SOLIDS_OUTSIDE_REGIONS;
    const static VerificationValidationTest ALL_BOTS_VOLUME_MODE;
    const static VerificationValidationTest NO_BOTS_LH_ORIENT; // TODO: this command can run faster if use unix
    const static VerificationValidationTest ALL_REGIONS_MAT;
    const static VerificationValidationTest ALL_REGIONS_LOS;
    const static VerificationValidationTest NO_NULL_REGIONS;
    const static VerificationValidationTest NO_OVERLAPS;
    const static VerificationValidationTest NO_DUPLICATE_ID;
    const static VerificationValidationTest NO_MATRICES;
    const static VerificationValidationTest NO_INVALID_AIRCODE_REGIONS;
    const static std::vector<VerificationValidationTest> allTests;
    // TODO: missing "No errors when top level drawn"
    // TODO: missing "BoTs are valid"
    // TODO: missing "Air does not stick out"
    // TODO: missing "Title"
    // TODO: missing "Ground plane at z=0"
};
#endif