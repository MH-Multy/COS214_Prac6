#ifndef TESTSYSTEM_H
#define TESTSYSTEM_H

#include <memory>
#include <string>

#include "CommandInvoker.h"
#include "DefaultWorkflowMediator.h"
#include "IndustrySetup.h"
#include "WorkflowEngine.h"
#include "WorkflowFacade.h"
#include "WorkflowRequest.h"

/** Participant ids from IndustrySetup.cpp. */
namespace people {
const int SARAH = 101;    // Researcher
const int THABO = 102;    // Researcher, not cleared to run tests
const int NAIDOO = 103;   // HeadResearcher (lab approver)
const int LERATO = 104;   // LabTechnician
const int NOMVULA = 201;  // Applicant
const int JOHAN = 202;    // Applicant, not eligible
const int AYESHA = 203;   // CreditAnalyst
const int PIETER = 204;   // CreditManager (loan approver) and Applicant
}  // namespace people

/** Task ids produced by the factories for the configured stages. */
namespace labTask {
const int RESERVE = 3, PREPARE = 4, RUN = 6, RECORD = 7, REVIEW = 9;
}
namespace loanTask {
const int CREDIT = 3, AFFORD = 4, OFFER = 6, ACCEPT = 7, DISBURSE = 8, SIGNOFF = 10;
}

/**
 * @brief The whole system wired exactly as main() does, with both industries configured.
 */
struct TestSystem {
    std::shared_ptr<WorkflowEngine> engine;
    std::shared_ptr<CommandInvoker> invoker;
    std::shared_ptr<DefaultWorkflowMediator> mediator;
    WorkflowFacade facade;

    /** @param labTimeoutSeconds Lab auto-approval timeout; large by default so nothing auto-approves. */
    explicit TestSystem(int labTimeoutSeconds = 3600)
        : engine(std::make_shared<WorkflowEngine>()),
          invoker(std::make_shared<CommandInvoker>()),
          mediator(std::make_shared<DefaultWorkflowMediator>(*engine)),
          facade(engine, invoker, mediator) {
        configureAllIndustries(facade, labTimeoutSeconds);
    }

    int labRequest(int requester = people::SARAH, const std::string& risk = "2") {
        WorkflowRequest r("Blood panel", requester);
        r.setAttribute("testType", "Blood panel");
        r.setAttribute("equipment", "Centrifuge");
        r.setAttribute("riskLevel", risk);
        return facade.submitRequest("Lab Test Request", r);
    }

    int loanRequest(int requester = people::NOMVULA, const std::string& amount = "120000") {
        WorkflowRequest r("Home renovation loan", requester);
        r.setAttribute("loanAmount", amount);
        r.setAttribute("termMonths", "48");
        r.setAttribute("monthlyIncome", "28000");
        return facade.submitRequest("Loan Application", r);
    }

    int approvedLab() {
        int id = labRequest();
        facade.approveRequest(id, people::NAIDOO);
        return id;
    }

    int approvedLoan() {
        int id = loanRequest();
        facade.approveRequest(id, people::PIETER);
        return id;
    }

    /** Whether the instance's history contains an event of @p type. */
    bool hasEvent(int id, const std::string& type) const {
        std::vector<WorkflowEvent> h = facade.getHistory(id);
        for (std::size_t i = 0; i < h.size(); ++i)
            if (h[i].getType() == type) return true;
        return false;
    }
};

#endif
