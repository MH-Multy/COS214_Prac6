#include "LegacyRecordSystem.h"
#include <string>
#include <sstream>
#include <map>
#include <vector>

LegacyRecordSystem::LegacyRecordSystem() {
    // Seed some legacy mock records
    // ID 101: Dr. Sarah Khumalo
    legacyDb[101] = {"ROLE_STAFF", "BIOSAFETY_LEVEL_2", "LAB_EQUIPMENT_RESERVATION"};
    // ID 102: Student Intern
    legacyDb[102] = {"ROLE_INTERN", "LAB_EQUIPMENT_RESERVATION"};
}

std::string LegacyRecordSystem::queryRecord(int recordId) const {
    auto it = legacyDb.find(recordId);
    if (it != legacyDb.end()) {
        return "STATUS:ACTIVE;ID:" + std::to_string(recordId);
    }
    return "STATUS:NOT_FOUND";
}

std::string LegacyRecordSystem::lookupPermissions(int recordId) const {
    auto it = legacyDb.find(recordId);
    if (it == legacyDb.end()) {
        return "";
    }

    std::string permissionsStr;
    for (size_t i = 0; i < it->second.size(); ++i) {
        permissionsStr += it->second[i];
        if (i + 1 < it->second.size()) {
            permissionsStr += ",";
        }
    }
    return permissionsStr;
}

void LegacyRecordSystem::insertRecord(int recordId, const std::vector<std::string>& legacyRoles) {
    legacyDb[recordId] = legacyRoles;
}