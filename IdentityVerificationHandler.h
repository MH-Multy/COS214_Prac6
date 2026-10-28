#ifndef IDENTITY_VERIFICATION_HANDLER_H
#define IDENTITY_VERIFICATION_HANDLER_H

#include "ValidationHandler.h"
#include "OrganizationDirectory.h"
#include <memory>
#include <string>
/**
 * @addtogroup ChainOfResponsibility
 * @{
 */
/**
 * @brief ConcreteHandler that verifies whether the acting participant 
 *        exists in the organization directory (Adapter target interface).
 */
class IdentityVerificationHandler : public ValidationHandler {
private:
    std::shared_ptr<OrganizationDirectory> directory;

protected:
    /**
     * @brief Evaluates whether the current actor ID in wf is registered.
     */
    ValidationResult check(const WorkflowInstance& wf) override;

    /**
     * @brief Selective participation hook.
     *        Only processes if the workflow instance specifies an actor ID.
     */
    bool canHandle(const WorkflowInstance& wf) const override;

    std::string getHandlerName() const override;

public:
    /**
     * @param dir Shared pointer to the OrganizationDirectory interface.
     */
    explicit IdentityVerificationHandler(std::shared_ptr<OrganizationDirectory> dir);
    virtual ~IdentityVerificationHandler() = default;

    std::shared_ptr<OrganizationDirectory> getDirectory() const;
};
/**
 * @}
 */
#endif // IDENTITY_VERIFICATION_HANDLER_H