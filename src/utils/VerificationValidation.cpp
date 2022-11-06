//
// Created by isaacy13 on 10/21/2022.
//

#include "VerificationValidation.h"

using Test = VerificationValidation::Test;
using DefaultTests = VerificationValidation::DefaultTests;

const Test DefaultTests::MISMATCHED_DUP_IDS          = {"No mis-matched duplicate IDs", "lc", "File", "lc",  false, {Arg("-m", false, NULL), Arg("all", false, NULL)}};
const Test DefaultTests::NO_DUPLICATE_ID             = {"Duplicate ID check", "lc", "File", "lc",  false, {Arg("-d", false, NULL), Arg("all", false, NULL)}};
const Test DefaultTests::NO_NULL_REGIONS             = {"No null region", "gqa", "General", "gqa", true, {Arg("-Ao", false, NULL), Arg("-g", true, "4mm,4mm"), Arg("-t", true, "0.3mm"), Arg("all", false, NULL)}};
const Test DefaultTests::NO_OVERLAPS                 = {"Overlaps cleaned to gridsize with tolerance", "gqa", "General", "gqa", true, {Arg("-Ao", false, NULL), Arg("-g", true, "32mm,4mm"), Arg("-t", true, "0.3mm"), Arg("all", false, NULL)}};
const Test DefaultTests::NO_NESTED_REGIONS           = {"No nested regions", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("-type region -below -type region", false, NULL)}};
const Test DefaultTests::NO_EMPTY_COMBOS             = {"No empty combos", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("-nnodes 0", false, NULL)}};
const Test DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS   = {"No solids outside of regions", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("! -below -type region -type shape", false, NULL)}};
const Test DefaultTests::ALL_BOTS_VOLUME_MODE        = {"All BoTs are volume mode (should return nothing)", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("-type bot ! -type volume", false, NULL)}};
// TODO: this command can run faster if use unix
const Test DefaultTests::NO_BOTS_LH_ORIENT           = {"No BoTs are left hand orientation", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("-type bot -param orient=lh", false, NULL)}};
const Test DefaultTests::ALL_REGIONS_MAT             = {"All regions have material", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("-type region ! -attr aircode ! -attr material_id", false, NULL)}};
const Test DefaultTests::ALL_REGIONS_LOS             = {"All regions have LOS", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("-type region ! -attr aircode ! -attr los", false, NULL)}};
const Test DefaultTests::NO_MATRICES                 = {"No matrices", "search", "File", "search", false, {Arg("/all", false, NULL), Arg("-! -matrix IDN", false, NULL)}};
const Test DefaultTests::NO_INVALID_AIRCODE_REGIONS  = {"No regions have aircodes (except actual air regions)", "search", "General", "search", false, {Arg("/all", false, NULL), Arg("-type region -attr aircode", false, NULL)}};
const Test DefaultTests::VALID_TITLE                 = {"Valid title", "title", "General", "title", false, {}};

const std::vector<Test> DefaultTests::allTests = {
    DefaultTests::MISMATCHED_DUP_IDS,
    DefaultTests::NO_DUPLICATE_ID,
    DefaultTests::NO_NULL_REGIONS,
    DefaultTests::NO_OVERLAPS,
    DefaultTests::NO_NESTED_REGIONS,
    DefaultTests::NO_EMPTY_COMBOS,
    DefaultTests::NO_SOLIDS_OUTSIDE_REGIONS,
    DefaultTests::ALL_BOTS_VOLUME_MODE,
    DefaultTests::NO_BOTS_LH_ORIENT,
    DefaultTests::ALL_REGIONS_MAT,
    DefaultTests::ALL_REGIONS_LOS,
    DefaultTests::NO_MATRICES,
    DefaultTests::NO_INVALID_AIRCODE_REGIONS,
    DefaultTests::VALID_TITLE
};

const std::map<QString, VerificationValidation::Test> DefaultTests::nameToTestMap = {
    std::make_pair("No null region", DefaultTests::NO_NULL_REGIONS),
    std::make_pair("Overlaps cleaned to gridsize with tolerance", DefaultTests::NO_OVERLAPS)
};