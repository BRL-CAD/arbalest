//
// Created by isaacy13 on 10/21/2022.
//

#include "VerificationValidation.h"

const VerificationValidationTest VerificationValidationDefaultTests::MISMATCHED_DUP_IDS          = {"No mis-matched duplicate IDs", "lc -m all", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_NESTED_REGIONS           = {"No nested regions", "search /all -type region -below -type region", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_EMPTY_COMBOS             = {"No empty combos", "search /all -nnodes 0", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_SOLIDS_OUTSIDE_REGIONS   = {"No solids outside of regions", "search /all ! -below -type region -type shape", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::ALL_BOTS_VOLUME_MODE        = {"All BoTs are volume mode (should return nothing)", "search all -type bot ! -type volume", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_BOTS_LH_ORIENT           = {"No BoTs are left hand orientation", "search all -type bot -param orient=lh", "General"}; // TODO: this command can run faster if use unix
const VerificationValidationTest VerificationValidationDefaultTests::ALL_REGIONS_MAT             = {"All regions have material", "search /all -type region ! -attr aircode ! -attr material_id", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::ALL_REGIONS_LOS             = {"All regions have LOS", "search /all -type region ! -attr aircode ! -attr los", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_NULL_REGIONS             = {"No null region", "gqa -Ao -g4mm,4mm -t0.3mm all", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_OVERLAPS                 = {"Overlaps cleaned to 4mm gridsize with 0.3mm tolerance", "gqa -Ao -g32mm,4mm -t0.3mm all", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_DUPLICATE_ID             = {"Duplicate ID check", "lc -d all","General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_MATRICES                 = {"No matrices", "search /all ! -matrix IDN", "General"};
const VerificationValidationTest VerificationValidationDefaultTests::NO_INVALID_AIRCODE_REGIONS  = {"No regions have aircodes (except actual air regions)", "search /all -type region -attr aircode", "General"};

const std::vector<VerificationValidationTest> VerificationValidationDefaultTests::allTests = {
    VerificationValidationDefaultTests::MISMATCHED_DUP_IDS,
    VerificationValidationDefaultTests::NO_NESTED_REGIONS,
    VerificationValidationDefaultTests::NO_EMPTY_COMBOS,
    VerificationValidationDefaultTests::NO_SOLIDS_OUTSIDE_REGIONS,
    VerificationValidationDefaultTests::ALL_BOTS_VOLUME_MODE,
    VerificationValidationDefaultTests::NO_BOTS_LH_ORIENT,
    VerificationValidationDefaultTests::ALL_REGIONS_MAT,
    VerificationValidationDefaultTests::ALL_REGIONS_LOS,
    VerificationValidationDefaultTests::NO_NULL_REGIONS,
    VerificationValidationDefaultTests::NO_OVERLAPS,
    VerificationValidationDefaultTests::NO_DUPLICATE_ID,
    VerificationValidationDefaultTests::NO_MATRICES,
    VerificationValidationDefaultTests::NO_INVALID_AIRCODE_REGIONS
};