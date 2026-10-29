/**
 * @defgroup Adapter
 * @brief Subsystem bridging modern directory queries to legacy record systems.
 * @{
 * @}
 */

#ifndef ORGANIZATION_DIRECTORY_H
#define ORGANIZATION_DIRECTORY_H

#include <string>
/**
 * @addtogroup Adapter
 * @{
 */
/**
 * @brief Target Interface in the GoF Adapter Pattern.
 *        Defines standard modern operations required by the workflow engine.
 */
class OrganizationDirectory {
public:
    virtual ~OrganizationDirectory() = default;
    /**
     * @brief Check whether an actor exists in the organization registry.
     */
    virtual bool userExists(int id) const = 0;
    /**
     * @brief Check whether an actor is authorized for a specific activity.
     */
    virtual bool isAuthorized(int id, const std::string& activity) const = 0;
};
/**
 * @}
 */
#endif // ORGANIZATION_DIRECTORY_H