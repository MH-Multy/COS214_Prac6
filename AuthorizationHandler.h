#ifndef AUTHORIZATION_HANDLER_H
#define AUTHORIZATION_HANDLER_H

#include "ValidationHandler.h"
#include "OrganizationDirectory.h"
#include <memory>
#include <string>

/**
 * @brief ConcreteHandler verifying whether an acting participant is authorized
 *        to perform a specific activity using the OrganizationDirectory target interface.
 */
class AuthorizationHandler : public ValidationHandler {
private:
    std::shared_ptr<OrganizationDirectory> directory;
    std::string activity;

protected:
    /**
     * @brief Performs the actual authorization check against directory->isAuthorized().
     */
    ValidationResult check(const WorkflowInstance& wf) override;

    /**
     * @brief Selective participation hook.
     *        Only executes if an activity is configured and an actor is present.
     */
    bool canHandle(const WorkflowInstance& wf) const override;

    std::string getHandlerName() const override;

public:
    /**
     * @param dir Shared pointer to the OrganizationDirectory interface.
     * @param act The activity or permission string required (e.g., "LAB_EQUIPMENT_RESERVATION").
     */
    AuthorizationHandler(std::shared_ptr<OrganizationDirectory> dir, std::string act);
    virtual ~AuthorizationHandler() = default;
    const std::string& getActivity() const;
    void setActivity(const std::string& act);
    std::shared_ptr<OrganizationDirectory> getDirectory() const;
};

#endif // AUTHORIZATION_HANDLER_H