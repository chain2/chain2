#include "utilblock.h"
#include "chain.h"

static std::string BlockValidityStr(BlockStatus s) {
    std::vector<std::string> checks;

    uint32_t validstate = s & BlockStatus::BLOCK_VALID_MASK;

    switch (validstate) {
        case 5:
            checks.push_back("Valid scripts and signatures");
            // fallthrough
        case 4:
            checks.push_back("Valid coin spends");
            // fallthrough
        case 3:
            checks.push_back("Valid transactions");
            // fallthrough
        case 2:
            checks.push_back("Parent headers are valid");
            // fallthrough
        case 1:
            checks.push_back("Valid header");
            break;
        default:
            checks.push_back("UNKNOWN");
    };


    std::string res;
    for (auto c = checks.rbegin(); c != checks.rend(); ++c) {
        if (!res.empty()) {
            res += "; ";
        }
        res += *c;
    }
    return res;
}

static std::string BlockFailureStr(BlockStatus s) {
    std::string res;
    if (s & BlockStatus::BLOCK_FAILED_VALID) {
        res = "Block failed a validity check";
    }
    if (s & BlockStatus::BLOCK_FAILED_CHILD) {
        if (!res.empty()) {
            res += "; ";
        }
        res += "Block descends from a failed block";
    }
    return res;
}

static std::string BlockDataAvailStr(BlockStatus s) {
    std::string res;
    if (s & BlockStatus::BLOCK_HAVE_DATA) {
        res = "Block data available";
    }
    if (s & BlockStatus::BLOCK_FAILED_CHILD) {
        if (!res.empty()) {
            res += "; ";
        }
        res += "Undo data available";
    }
    return res;
}

std::tuple<std::string, std::string, std::string> BlockStatusToStr(uint32_t status) {
    BlockStatus s = BlockStatus(status);
    return std::make_tuple(BlockValidityStr(s),
                           BlockFailureStr(s),
                           BlockDataAvailStr(s));
}
