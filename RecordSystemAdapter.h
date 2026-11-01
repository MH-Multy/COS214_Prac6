#ifndef RECORD_SYSTEM_ADAPTER_H
#define RECORD_SYSTEM_ADAPTER_H

#include "OrganizationDirectory.h"
#include "LegacyRecordSystem.h"
#include <memory>

/**
 * @brief Adapter in the GoF Adapter Pattern.
 *        Adapts LegacyRecordSystem to fit the OrganizationDirectory interface.
 */
class RecordSystemAdapter : public OrganizationDirectory {
private:
    std::shared_ptr<LegacyRecordSystem> legacySystem;

public:
    explicit RecordSystemAdapter(std::shared_ptr<LegacyRecordSystem> legacy);
    virtual ~RecordSystemAdapter() = default;

    // OrganizationDirectory implementation
    bool userExists(int id) const override;
    bool isAuthorized(int id, const std::string& activity) const override;
};

#endif // RECORD_SYSTEM_ADAPTER_H