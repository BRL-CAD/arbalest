//
// Created by isaacy13 on 10/21/2022.
//

#include "VerificationValidation.h"

using Test = VerificationValidation::Test;
using DefaultTests = VerificationValidation::DefaultTests;

Test DefaultTests::MISMATCHED_DUP_IDS          = {"No mis-matched duplicate IDs", "lc", "File", "lc",  false, {Arg("-m"), Arg("$OBJECT", NULL, Arg::Type::ObjectName)}};
Test DefaultTests::NO_DUPLICATE_ID             = {"Duplicate ID check", "lc", "File", "lc",  false, {Arg("-d"), Arg("$OBJECT", NULL, Arg::Type::ObjectName)}};
Test DefaultTests::NO_NULL_REGIONS             = {"No null region", "gqa", "General", "gqa", true, {Arg("-Ao"), Arg("-g", "4mm,4mm"), Arg("-t", "0.3mm"), Arg("$OBJECT", NULL, Arg::Type::ObjectName)}};
Test DefaultTests::NO_OVERLAPS                 = {"Overlaps cleaned to gridsize with tolerance", "gqa", "General", "gqa", true, {Arg("-Ao"), Arg("-g", "32mm,4mm"), Arg("-t", "0.3mm"), Arg("$OBJECT", NULL, Arg::Type::ObjectName)}};
Test DefaultTests::NO_NESTED_REGIONS           = {"No nested regions", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region -below -type region")}};
Test DefaultTests::NO_EMPTY_COMBOS             = {"No empty combos", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-nnodes 0")}};
Test DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS   = {"No solids outside of regions", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("! -below -type region -type shape")}};
Test DefaultTests::ALL_BOTS_VOLUME_MODE        = {"All BoTs are volume mode (should return nothing)", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type bot ! -type volume")}};
// TODO: this command can run faster if use unix
Test DefaultTests::NO_BOTS_LH_ORIENT           = {"No BoTs are left hand orientation", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type bot -param orient=lh")}};
Test DefaultTests::ALL_REGIONS_MAT             = {"All regions have material", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region ! -attr aircode ! -attr material_id")}};
Test DefaultTests::ALL_REGIONS_LOS             = {"All regions have LOS", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region ! -attr aircode ! -attr los")}};
Test DefaultTests::NO_MATRICES                 = {"No matrices", "search", "File", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-! -matrix IDN")}};
Test DefaultTests::NO_INVALID_AIRCODE_REGIONS  = {"No regions have aircodes (except actual air regions)", "search", "General", "search", false, {Arg("/$OBJECT", NULL, Arg::Type::ObjectPath), Arg("-type region -attr aircode")}};
Test DefaultTests::VALID_TITLE                 = {"Valid title", "title", "General", "title", false, {}};

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