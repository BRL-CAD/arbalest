//
// Created by isaacy13 on 10/21/2022.
//

#include "VerificationValidation.h"

using Test = VerificationValidation::Test;
using DefaultTests = VerificationValidation::DefaultTests;

const Test DefaultTests::MISMATCHED_DUP_IDS          = {"No mis-matched duplicate IDs", "lc -m all", "File"};
const Test DefaultTests::NO_NESTED_REGIONS           = {"No nested regions", "search /all -type region -below -type region", "General"};
const Test DefaultTests::NO_EMPTY_COMBOS             = {"No empty combos", "search /all -nnodes 0", "General"};
const Test DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS   = {"No solids outside of regions", "search /all ! -below -type region -type shape", "General"};
const Test DefaultTests::ALL_BOTS_VOLUME_MODE        = {"All BoTs are volume mode (should return nothing)", "search all -type bot ! -type volume", "General"};
const Test DefaultTests::NO_BOTS_LH_ORIENT           = {"No BoTs are left hand orientation", "search all -type bot -param orient=lh", "General"}; // TODO: this command can run faster if use unix
const Test DefaultTests::ALL_REGIONS_MAT             = {"All regions have material", "search /all -type region ! -attr aircode ! -attr material_id", "General"};
const Test DefaultTests::ALL_REGIONS_LOS             = {"All regions have LOS", "search /all -type region ! -attr aircode ! -attr los", "General"};
const Test DefaultTests::NO_NULL_REGIONS             = {"No null region", "gqa -Ao -g4mm,4mm -t0.3mm all", "General"};
const Test DefaultTests::NO_OVERLAPS                 = {"Overlaps cleaned to 4mm gridsize with 0.3mm tolerance", "gqa -Ao -g32mm,4mm -t0.3mm all", "General"};
const Test DefaultTests::NO_DUPLICATE_ID             = {"Duplicate ID check", "lc -d all","File"};
const Test DefaultTests::NO_MATRICES                 = {"No matrices", "search /all ! -matrix IDN", "File"};
const Test DefaultTests::NO_INVALID_AIRCODE_REGIONS  = {"No regions have aircodes (except actual air regions)", "search /all -type region -attr aircode", "General"};
const Test DefaultTests::VALID_TITLE                 = {"Valid title", "title", "General"};

const std::vector<Test> DefaultTests::allTests = {
    DefaultTests::MISMATCHED_DUP_IDS,
    DefaultTests::NO_NESTED_REGIONS,
    DefaultTests::NO_EMPTY_COMBOS,
    DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS,
    DefaultTests::ALL_BOTS_VOLUME_MODE,
    DefaultTests::NO_BOTS_LH_ORIENT,
    DefaultTests::ALL_REGIONS_MAT,
    DefaultTests::ALL_REGIONS_LOS,
    DefaultTests::NO_NULL_REGIONS,
    DefaultTests::NO_OVERLAPS,
    DefaultTests::NO_DUPLICATE_ID,
    DefaultTests::NO_MATRICES,
    DefaultTests::NO_INVALID_AIRCODE_REGIONS,
    DefaultTests::VALID_TITLE
};