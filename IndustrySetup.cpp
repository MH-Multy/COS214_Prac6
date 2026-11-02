/**
 * @file IndustrySetup.cpp
 * @brief Application configuration for the laboratory and loan industries, and custom workflows.
 *
 * This is the only file that knows which industries exist. Each one is assembled
 * from generic engine classes: a records system behind the Adapter, a Chain of
 * Responsibility of reusable validation handlers, stages for the Composite work
 * tree, and a factory choosing the approval regime. No engine class changes.
 */
#include "IndustrySetup.h"

#include <memory>
#include <stdexcept>

#include "AuthorizationHandler.h"
#include "IdentityVerificationHandler.h"
#include "LegacyRecordSystem.h"
#include "Participant.h"
#include "RecordSystemAdapter.h"
#include "RequiredAttributesHandler.h"
#include "ThresholdHandler.h"
#include "WorkflowDefinition.h"
#include "WorkflowFacade.h"
#include "WorkflowFactory.h"

namespace {
std::vector<std::string> list(const char* a, const char* b = 0, const char* c = 0) {
    std::vector<std::string> v;
    v.push_back(a);
    if (b) v.push_back(b);
    if (c) v.push_back(c);
    return v;
}
}  // namespace

void configureLabTesting(WorkflowFacade& facade, int approvalTimeoutSeconds) {
    // The laboratory's staff records system (Adapter adaptee). Records 101 and 102 are
    // already loaded by LegacyRecordSystem; the head researcher and technician are added here.
    std::shared_ptr<LegacyRecordSystem> lims = std::make_shared<LegacyRecordSystem>();
    lims->insertRecord(103, list("HEAD_RESEARCHER", "BIOSAFETY_LEVEL_2", "LAB_EQUIPMENT_RESERVATION"));
    lims->insertRecord(104, list("LAB_TECH", "LAB_EQUIPMENT_RESERVATION"));
    std::shared_ptr<OrganizationDirectory> directory = std::make_shared<RecordSystemAdapter>(lims);

    facade.registerParticipant(Participant(101, "Dr Sarah Khumalo", list("Researcher")));
    facade.registerParticipant(Participant(102, "Thabo Mokoena", list("Researcher")));
    facade.registerParticipant(Participant(103, "Prof Anil Naidoo", list("HeadResearcher")));
    facade.registerParticipant(Participant(104, "Lerato Molefe", list("LabTechnician")));

    // Validation rules, in order (Chain of Responsibility).
    std::shared_ptr<ValidationHandler> chain =
        std::make_shared<RequiredAttributesHandler>(list("testType", "equipment", "riskLevel"));
    chain->setNext(std::make_shared<IdentityVerificationHandler>(directory))
        ->setNext(std::make_shared<AuthorizationHandler>(directory, "BIOSAFETY_LEVEL_2"))
        ->setNext(std::make_shared<ThresholdHandler>("riskLevel", 3));

    std::shared_ptr<WorkflowDefinition> def = std::make_shared<WorkflowDefinition>("Lab Test Request", 1);
    def->setDescription("Request, approve, run and log a laboratory test")
        .setApproverRole("HeadResearcher")
        .setApprovalTimeout(approvalTimeoutSeconds)
        .addRequestField("testType", "Test type (e.g. Blood panel)")
        .addRequestField("equipment", "Equipment needed (e.g. Centrifuge)")
        .addRequestField("riskLevel", "Risk level 1-5 (at most 3 allowed)")
        .addStage("Preparation", list("Reserve equipment (LabTechnician)", "Prepare samples (Requester)"), false)
        .addStage("Testing", list("Run test (Requester)", "Record results (Requester)"), true)
        .addStage("Analysis", list("Peer review of results (Researcher)"), true)
        .setValidationChain(chain)
        .setFactory(std::make_shared<AutoApprovalComponentFactory>());
    facade.registerWorkflow(def);
}

void configureLoanApplications(WorkflowFacade& facade) {
    // The bank's customer and staff records system (a second, separate adaptee).
    std::shared_ptr<LegacyRecordSystem> kyc = std::make_shared<LegacyRecordSystem>();
    kyc->insertRecord(201, list("CUSTOMER", "APPLY_PERSONAL_LOAN"));
    kyc->insertRecord(202, list("CUSTOMER"));                       // not eligible for this product
    kyc->insertRecord(203, list("CREDIT_ANALYST"));
    kyc->insertRecord(204, list("CREDIT_MANAGER", "APPLY_PERSONAL_LOAN"));
    std::shared_ptr<OrganizationDirectory> directory = std::make_shared<RecordSystemAdapter>(kyc);

    facade.registerParticipant(Participant(201, "Nomvula Dlamini", list("Applicant")));
    facade.registerParticipant(Participant(202, "Johan Botha", list("Applicant")));
    facade.registerParticipant(Participant(203, "Ayesha Patel", list("CreditAnalyst")));
    facade.registerParticipant(Participant(204, "Pieter van Wyk", list("CreditManager", "Applicant")));

    std::shared_ptr<ValidationHandler> chain =
        std::make_shared<RequiredAttributesHandler>(list("loanAmount", "termMonths", "monthlyIncome"));
    chain->setNext(std::make_shared<IdentityVerificationHandler>(directory))
        ->setNext(std::make_shared<AuthorizationHandler>(directory, "APPLY_PERSONAL_LOAN"))
        ->setNext(std::make_shared<ThresholdHandler>("loanAmount", 250000));

    std::shared_ptr<WorkflowDefinition> def = std::make_shared<WorkflowDefinition>("Loan Application", 1);
    def->setDescription("Apply for, assess, approve and pay out a personal loan")
        .setApproverRole("CreditManager")
        .addRequestField("loanAmount", "Loan amount in rand (at most 250000)")
        .addRequestField("termMonths", "Term in months (e.g. 36)")
        .addRequestField("monthlyIncome", "Monthly income in rand")
        .addStage("Assessment", list("Credit bureau check (CreditAnalyst)", "Affordability assessment (CreditAnalyst)"), true)
        .addStage("Offer and payout", list("Prepare loan offer (CreditAnalyst)", "Applicant accepts offer (Requester)",
                                           "Disburse funds (CreditManager)"), true)
        .setValidationChain(chain)
        .setFactory(std::make_shared<ManualReviewComponentFactory>());
    facade.registerWorkflow(def);
}

void configureAllIndustries(WorkflowFacade& facade, int labTimeoutSeconds) {
    configureLabTesting(facade, labTimeoutSeconds);
    configureLoanApplications(facade);
}

void configureCustomWorkflow(WorkflowFacade& facade, const std::string& name, const std::string& description,
                             const std::string& approverRole, int autoApproveSeconds,
                             const std::vector<std::string>& fields, const std::string& limitField, double limitMax,
                             const std::vector<std::pair<std::string, std::vector<std::string>>>& stages,
                             const std::vector<bool>& sequential) {
    if (name.empty()) throw std::invalid_argument("The workflow needs a name");
    std::vector<std::string> types = facade.listWorkflowTypes();
    for (std::size_t i = 0; i < types.size(); ++i)
        if (types[i] == name) throw std::invalid_argument("A workflow called '" + name + "' already exists");
    bool anyTask = false;
    for (std::size_t s = 0; s < stages.size(); ++s) anyTask = anyTask || !stages[s].second.empty();
    if (!anyTask) throw std::invalid_argument("The workflow needs at least one task");

    // A records system for this organisation, holding everyone currently registered (Adapter reuse).
    std::shared_ptr<LegacyRecordSystem> records = std::make_shared<LegacyRecordSystem>();
    std::vector<Participant> people = facade.listParticipants();
    for (std::size_t i = 0; i < people.size(); ++i) records->insertRecord(people[i].getId(), list("STAFF"));
    std::shared_ptr<OrganizationDirectory> directory = std::make_shared<RecordSystemAdapter>(records);

    std::shared_ptr<ValidationHandler> chain = std::make_shared<RequiredAttributesHandler>(fields);
    std::shared_ptr<ValidationHandler> last = chain->setNext(std::make_shared<IdentityVerificationHandler>(directory));
    if (!limitField.empty()) last->setNext(std::make_shared<ThresholdHandler>(limitField, limitMax));

    std::shared_ptr<WorkflowDefinition> def = std::make_shared<WorkflowDefinition>(name, 1);
    def->setDescription(description).setApproverRole(approverRole).setValidationChain(chain);
    for (std::size_t i = 0; i < fields.size(); ++i) def->addRequestField(fields[i], fields[i]);
    for (std::size_t s = 0; s < stages.size(); ++s)
        if (!stages[s].second.empty())
            def->addStage(stages[s].first, stages[s].second, s < sequential.size() ? sequential[s] : true);
    if (autoApproveSeconds >= 0) {
        def->setApprovalTimeout(autoApproveSeconds).setFactory(std::make_shared<AutoApprovalComponentFactory>());
    } else {
        def->setFactory(std::make_shared<ManualReviewComponentFactory>());
    }
    facade.registerWorkflow(def);
}
