#pragma once
#include <set>
#include <vector>
#include <cstdint>

static const std::set<uint32_t> essentialNpcEntries = {
    6930,  // Innkeeper
    28690, // Stable Master
    9858,  // Auctioneer Kresky
    8719,  // Auctioneer Fitch
    9856,  // Auctioneer Grimful
    6491   // Spirit Healer
};

static const std::set<uint32_t> classTrainerEntries = {
    26327, 26324, 26325, 26326, 26328, 26329, 26330, 26331, 26332, 29195};

static const std::set<uint32_t> professionTrainerEntries = {
    2836, 8128, 8736, 19187, 19180, 19052, 908, 2627, 19184, 2834, 19185};

static const std::set<uint32_t> vendorEntries = {
    28692, 28776, 4255, 29636, 29493, 2622,
    32478, // Fishing Vendor
    500033, 500034, 500035, 500036, 500037};

static const std::set<uint32_t> objectEntries = {
    1685, 4087, 180715, 2728, 184137, 191028};

static const std::set<uint32_t> portalEntries = {
    500000, 500001, 500002, 500003, 500004, 500005, 500006, 500007, 500008, 500009};