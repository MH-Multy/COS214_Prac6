#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H
#include <memory>
#include "WorkItem.h"

/** ABSTRACT FACTORY. The engine only ever sees this interface, so it can
 *  never mix a lab item with a loan router. */
class WorkflowComponentFactory {
public:
    virtual ~WorkflowComponentFactory() {}
    virtual std::unique_ptr<WorkItem> createWorkItem(const std::string& name) const = 0;
    virtual std::unique_ptr<Payload>  createPayload()  const = 0;
    virtual std::unique_ptr<TaskRouter> createRouter() const = 0;
};

/* ============ Family 1: laboratory testing ============ */
class LabTestWorkItem : public WorkItem {
public:
    LabTestWorkItem(const std::string& n) : WorkItem(n) {}
    bool requiresApproval() const { return true; }
    std::string resourceNeeded() const { return "Lab equipment"; }
    std::string industry() const { return "Laboratory"; }
};
class LabReportPayload : public Payload {
public:
    std::string describe() const { return "LabReport{risk, funding, daily log}"; }
};
class LabQualificationRouter : public TaskRouter {
public:
    std::string assign(const WorkItem&) const { return "qualified researcher"; }
};
class LabTestingComponentFactory : public WorkflowComponentFactory {
public:
    std::unique_ptr<WorkItem> createWorkItem(const std::string& n) const
        { return std::unique_ptr<WorkItem>(new LabTestWorkItem(n)); }
    std::unique_ptr<Payload> createPayload() const
        { return std::unique_ptr<Payload>(new LabReportPayload()); }
    std::unique_ptr<TaskRouter> createRouter() const
        { return std::unique_ptr<TaskRouter>(new LabQualificationRouter()); }
};

/* ============ Family 2: loan application ============ */
class LoanWorkItem : public WorkItem {
public:
    LoanWorkItem(const std::string& n) : WorkItem(n) {}
    bool requiresApproval() const { return true; }
    std::string resourceNeeded() const { return "Credit officer"; }
    std::string industry() const { return "Finance"; }
};
class LoanPayload : public Payload {
public:
    std::string describe() const { return "Loan{amount, collateral, credit score}"; }
};
class CreditLimitRouter : public TaskRouter {
public:
    std::string assign(const WorkItem&) const { return "credit officer within approval limit"; }
};
class LoanComponentFactory : public WorkflowComponentFactory {
public:
    std::unique_ptr<WorkItem> createWorkItem(const std::string& n) const
        { return std::unique_ptr<WorkItem>(new LoanWorkItem(n)); }
    std::unique_ptr<Payload> createPayload() const
        { return std::unique_ptr<Payload>(new LoanPayload()); }
    std::unique_ptr<TaskRouter> createRouter() const
        { return std::unique_ptr<TaskRouter>(new CreditLimitRouter()); }
};
#endif
