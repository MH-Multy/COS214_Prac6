#ifndef LEGACY_RECORD_SYSTEM_H
#define LEGACY_RECORD_SYSTEM_H

#include <string>
#include <map>
#include <vector>

/**
 * @brief Adaptee in the GoF Adapter Pattern.
 *        Represents an existing/legacy record-keeping system with incompatible interfaces.
 */
class LegacyRecordSystem {
private:
    std::map<int, std::vector<std::string>> legacyDb;

public:
    LegacyRecordSystem();
    virtual ~LegacyRecordSystem() = default;
    std::string queryRecord(int recordId) const;
    std::string lookupPermissions(int recordId) const;
    void insertRecord(int recordId, const std::vector<std::string>& legacyRoles);
};

#endif // LEGACY_RECORD_SYSTEM_H