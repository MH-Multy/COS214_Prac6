#include "RecordSystemAdapter.h"
#include <sstream>

RecordSystemAdapter::RecordSystemAdapter(std::shared_ptr<LegacyRecordSystem> legacy)
    : legacySystem(std::move(legacy)) {}

bool RecordSystemAdapter::userExists(int id) const {
    if (!legacySystem) {
        return false;
    }
    std::string response = legacySystem->queryRecord(id);
    return response.find("STATUS:ACTIVE") != std::string::npos;
}

bool RecordSystemAdapter::isAuthorized(int id, const std::string& activity) const {
    if (!legacySystem) {
        return false;
    }

    std::string legacyPerms = legacySystem->lookupPermissions(id);
    if (legacyPerms.empty()) {
        return false;
    }

    std::stringstream ss(legacyPerms);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (token == activity) {
            return true;
        }
    }

    return false;
}