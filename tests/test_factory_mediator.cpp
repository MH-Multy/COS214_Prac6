/** Tests for the Abstract Factory and Mediator patterns (owner: Lebo). */
#include "doctest.h"
#include "../src/WorkflowMediator.h"

namespace {

std::shared_ptr<WorkflowDefinition> makeLab() {
    std::shared_ptr<WorkflowDefinition> d(new WorkflowDefinition(
        "Lab test", std::shared_ptr<WorkflowFactory>(new ManualReviewComponentFactory()), "head researcher", 0));
    std::vector<std::string> keys; keys.push_back("sample"); keys.push_back("riskLevel");
    d->setValidationChain(std::shared_ptr<ValidationHandler>(new RequiredAttributesHandler(keys)));
    std::vector<std::string> intake(1, "Register sample"), testing;
    testing.push_back("Run assay"); testing.push_back("Record results");
    d->addStage("Intake", intake); d->addStage("Testing", testing);
    return d;
}

std::shared_ptr<WorkflowDefinition> makeLoan() {
    std::shared_ptr<WorkflowDefinition> d(new WorkflowDefinition(
        "Loan application", std::shared_ptr<WorkflowFactory>(new AutoApprovalComponentFactory()), "credit officer", 24));
    std::vector<std::string> keys(1, "amount");
    std::shared_ptr<ValidationHandler> h(new RequiredAttributesHandler(keys));
    h->setNext(std::shared_ptr<ValidationHandler>(new ThresholdHandler("amount", 500000)));
    d->setValidationChain(h);
    std::vector<std::string> checks; checks.push_back("Credit check"); checks.push_back("Income check");
    d->addStage("Assessment", checks);
    return d;
}

WorkflowRequest labRequest(bool complete) {
    WorkflowRequest r("Toxicology", 1);
    r.setAttribute("sample", "S-100");
    if (complete) r.setAttribute("riskLevel", "low");
    return r;
}

WorkflowRequest loanRequest(const std::string& amount) {
    WorkflowRequest r("Home loan", 3);
    r.setAttribute("amount", amount);
    return r;
}

struct Fixture {
    WorkflowEngine engine;
    ValidationService validator;
    ApprovalManager approvals;
    NotificationService notifier;
    DefaultWorkflowMediator mediator;
    Fixture() : mediator(engine, validator, approvals, notifier) {}
};

} // namespace

TEST_CASE("Abstract Factory: manual-review family is strict and sequential") {
    std::shared_ptr<WorkflowDefinition> def = makeLab();
    std::shared_ptr<ApprovalPolicy> policy = def->getFactory()->createApprovalPolicy(*def);
    std::shared_ptr<CompositeWorkItem> group = def->getFactory()->createTaskGroup(*def);

    CHECK(dynamic_cast<StrictApprovalPolicy*>(policy.get()) != 0);
    CHECK(group->isSequential());
    REQUIRE(group->getChildren().size() == 2);
    std::shared_ptr<CompositeWorkItem> stage =
        std::dynamic_pointer_cast<CompositeWorkItem>(group->getChildren()[1]);
    REQUIRE(stage);
    CHECK(stage->isSequential());
    CHECK(stage->getChildren().size() == 2);
}

TEST_CASE("Abstract Factory: auto-approval family uses timeout and parallel tasks") {
    std::shared_ptr<WorkflowDefinition> def = makeLoan();
    std::shared_ptr<ApprovalPolicy> policy = def->getFactory()->createApprovalPolicy(*def);
    std::shared_ptr<CompositeWorkItem> group = def->getFactory()->createTaskGroup(*def);

    CHECK(dynamic_cast<TimeoutApprovalPolicy*>(policy.get()) != 0);
    std::shared_ptr<CompositeWorkItem> stage =
        std::dynamic_pointer_cast<CompositeWorkItem>(group->getChildren()[0]);
    REQUIRE(stage);
    CHECK_FALSE(stage->isSequential());
}

TEST_CASE("Abstract Factory: instances never share a policy or task group") {
    Fixture f;
    std::shared_ptr<WorkflowDefinition> def = makeLab();
    int a = f.engine.startWorkflow(def, labRequest(true));
    int b = f.engine.startWorkflow(def, labRequest(true));
    std::shared_ptr<WorkflowInstance> wa = f.engine.getInstance(a);
    std::shared_ptr<WorkflowInstance> wb = f.engine.getInstance(b);

    CHECK(wa->getApprovalPolicy().get() != wb->getApprovalPolicy().get());
    CHECK(wa->getRoot().get() != wb->getRoot().get());

    wa->getRoot()->execute(1);
    CHECK(wa->getRoot()->isComplete());
    CHECK_FALSE(wb->getRoot()->isComplete());          // isolation
}

TEST_CASE("Strict policy: only the senior role may approve") {
    Fixture f;
    int id = f.engine.startWorkflow(makeLab(), labRequest(true));
    std::shared_ptr<WorkflowInstance> wf = f.engine.getInstance(id);
    std::vector<std::string> r1(1, "researcher"), r2(1, "head researcher");
    CHECK_FALSE(wf->getApprovalPolicy()->canApprove(*wf, Participant(1, "Thandi", r1)));
    CHECK(wf->getApprovalPolicy()->canApprove(*wf, Participant(2, "Dr Nkosi", r2)));
}

TEST_CASE("Mediator: valid lab request waits for a human, then starts work") {
    Fixture f;
    int id = f.engine.startWorkflow(makeLab(), labRequest(true));
    f.mediator.requestValidation(id);
    CHECK(f.engine.getInstance(id)->getStateName() == "PendingApproval");

    f.approvals.checkTimeouts();                        // strict policy never auto-approves
    CHECK(f.engine.getInstance(id)->getStateName() == "PendingApproval");

    f.mediator.approvalCompleted(id, true);
    CHECK(f.engine.getInstance(id)->getStateName() == "InProgress");
}

TEST_CASE("Mediator: failed validation rejects and notifies the requester") {
    Fixture f;
    int id = f.engine.startWorkflow(makeLab(), labRequest(false));   // riskLevel missing
    f.mediator.requestValidation(id);
    CHECK(f.engine.getInstance(id)->getStateName() == "Rejected");
    REQUIRE(f.notifier.getLog().size() == 1);
    CHECK(f.notifier.getLog()[0].find("riskLevel") != std::string::npos);
}

TEST_CASE("Mediator: chain stops a loan over the threshold") {
    Fixture f;
    int id = f.engine.startWorkflow(makeLoan(), loanRequest("900000"));
    f.mediator.requestValidation(id);
    CHECK(f.engine.getInstance(id)->getStateName() == "Rejected");
}

TEST_CASE("Mediator: loan auto-approves only after the timeout") {
    Fixture f;
    int id = f.engine.startWorkflow(makeLoan(), loanRequest("250000"));
    f.mediator.requestValidation(id);
    CHECK(f.engine.getInstance(id)->getStateName() == "PendingApproval");

    f.engine.getInstance(id)->advanceHours(23);
    f.approvals.checkTimeouts();
    CHECK(f.engine.getInstance(id)->getStateName() == "PendingApproval");   // too early

    f.engine.getInstance(id)->advanceHours(1);
    f.approvals.checkTimeouts();
    CHECK(f.engine.getInstance(id)->getStateName() == "InProgress");
}

TEST_CASE("Mediator: denied approval rejects the instance") {
    Fixture f;
    int id = f.engine.startWorkflow(makeLab(), labRequest(true));
    f.mediator.requestValidation(id);
    f.mediator.approvalCompleted(id, false);
    CHECK(f.engine.getInstance(id)->getStateName() == "Rejected");
}

TEST_CASE("Invalid operations are refused without changing state") {
    Fixture f;
    int id = f.engine.startWorkflow(makeLab(), labRequest(false));
    f.mediator.requestValidation(id);                   // -> Rejected
    std::shared_ptr<WorkflowInstance> wf = f.engine.getInstance(id);

    CHECK_FALSE(wf->approve(2));
    CHECK_FALSE(wf->start());
    f.mediator.approvalCompleted(id, true);             // must not resurrect it
    CHECK(wf->getStateName() == "Rejected");
}

TEST_CASE("Mediator ignores unknown workflow ids") {
    Fixture f;
    CHECK_NOTHROW(f.mediator.requestValidation(999));
    CHECK_NOTHROW(f.mediator.approvalCompleted(999, true));
}

TEST_CASE("Same engine and mediator serve both industries independently") {
    Fixture f;
    int lab = f.engine.startWorkflow(makeLab(), labRequest(true));
    int loan = f.engine.startWorkflow(makeLoan(), loanRequest("250000"));
    f.mediator.requestValidation(lab);
    f.mediator.requestValidation(loan);

    f.engine.getInstance(loan)->advanceHours(24);
    f.approvals.checkTimeouts();

    CHECK(f.engine.getInstance(loan)->getStateName() == "InProgress");
    CHECK(f.engine.getInstance(lab)->getStateName() == "PendingApproval");  // unaffected
}
