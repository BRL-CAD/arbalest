//
// Created by isaacy13 on 10/21/2022.
//

#include "VerificationValidation.h"

using Test = VerificationValidation::Test;
using DefaultTests = VerificationValidation::DefaultTests;

Test DefaultTests::MISMATCHED_DUP_IDS = Test("No mis-matched duplicate IDs", { "File" }, { Arg("lc"), Arg("-m"), Arg("$OBJECT", NULL, Arg::Type::ObjectName) });
Test DefaultTests::NO_DUPLICATE_ID = Test("Duplicate ID check", { "File" }, { Arg("lc"), Arg("-d"), Arg("$OBJECT", NULL, Arg::Type::ObjectName) });
Test DefaultTests::NO_NULL_REGIONS = Test("No null region", { "General" }, { Arg("gqa"), Arg("-Ao"), Arg("-g","4mm,4mm"), Arg("-t", "0.3mm"), Arg("$OBJECT", NULL, Arg::Type::ObjectName) });
Test DefaultTests::NO_OVERLAPS = Test("Overlaps cleaned to gridsize with tolerance", { "General" }, { Arg("gqa"), Arg("-Ao"), Arg("-g", "32mm,4mm"), Arg("-t", "0.3mm"), Arg("$OBJECT", NULL, Arg::Type::ObjectName) });
Test DefaultTests::NO_NESTED_REGIONS = Test("No nested regions", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region -below -type region") });
Test DefaultTests::NO_EMPTY_COMBOS = Test("No empty combos", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-nnodes 0") });
Test DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS = Test("No solids outside of regions", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("! -below -type region -type shape") });
Test DefaultTests::ALL_BOTS_VOLUME_MODE = Test("All BoTs are volume mode (should return nothing)", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type bot ! -type volume") });
// TODO: this command can run faster if use unix
Test DefaultTests::NO_BOTS_LH_ORIENT = Test("No BoTs are left hand orientation", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type bot -param orient=lh") });
Test DefaultTests::ALL_REGIONS_MAT = Test("All regions have material", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region ! -attr aircode ! -attr material_id") });
Test DefaultTests::ALL_REGIONS_LOS = Test("All regions have LOS", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region ! -attr aircode ! -attr los") });
Test DefaultTests::NO_MATRICES = Test("No matrices", { "File" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("! -matrix IDN") });
Test DefaultTests::NO_INVALID_AIRCODE_REGIONS = Test("No regions have aircodes (except actual air regions)", { "General" }, { Arg("search"), Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region -attr aircode") });
Test DefaultTests::VALID_TITLE = Test("Valid title", { "General" }, { Arg("title"), Arg("", NULL, Arg::Type::ObjectNone) });

const std::vector<Test*> DefaultTests::allTests = {
    &DefaultTests::MISMATCHED_DUP_IDS,
    &DefaultTests::NO_DUPLICATE_ID,
    &DefaultTests::NO_NULL_REGIONS,
    &DefaultTests::NO_OVERLAPS,
    &DefaultTests::NO_NESTED_REGIONS,
    &DefaultTests::NO_EMPTY_COMBOS,
    &DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS,
    &DefaultTests::ALL_BOTS_VOLUME_MODE,
    &DefaultTests::NO_BOTS_LH_ORIENT,
    &DefaultTests::ALL_REGIONS_MAT,
    &DefaultTests::ALL_REGIONS_LOS,
    &DefaultTests::NO_MATRICES,
    &DefaultTests::NO_INVALID_AIRCODE_REGIONS,
    &DefaultTests::VALID_TITLE
};

const std::map<QString, VerificationValidation::Test> DefaultTests::nameToTestMap = {
    std::make_pair("No null region", DefaultTests::NO_NULL_REGIONS),
    std::make_pair("Overlaps cleaned to gridsize with tolerance", DefaultTests::NO_OVERLAPS)
};