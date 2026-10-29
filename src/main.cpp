#include "Workflowmediator.h"

static void show(const WorkflowInstance& wf) {
    std::cout << "  state: " << wf.getStateName() << "\n";
}
int main() {
    WorkflowEngine engine;
    ValidationService vs; ApprovalManager am; NotificationService ns;
    DefaultWorkflowMediator mediator(engine, vs, am, ns);

    std::vector<std::string> researcherRole(1, "researcher"), headRole(1, "head researcher"), officerRole(1, "credit officer");
    Participant researcher(1, "Thandi", researcherRole), head(2, "Dr Nkosi", headRole), officer(3, "Pieter", officerRole);
    engine.registerParticipant(researcher); engine.registerParticipant(head); engine.registerParticipant(officer);

    // ---- Industry 1: laboratory testing (manual review family) ----
    std::shared_ptr<WorkflowDefinition> lab(new WorkflowDefinition(
        "Lab test", std::shared_ptr<WorkflowFactory>(new ManualReviewComponentFactory()), "head researcher", 0));
    std::vector<std::string> labKeys; labKeys.push_back("sample"); labKeys.push_back("riskLevel");
    lab->setValidationChain(std::shared_ptr<ValidationHandler>(new RequiredAttributesHandler(labKeys)));
    std::vector<std::string> intake(1, "Register sample"), testing;
    testing.push_back("Run assay"); testing.push_back("Record results");
    lab->addStage("Intake", intake); lab->addStage("Testing", testing);

    std::cout << "== LAB: valid request ==\n";
    WorkflowRequest r1("Toxicology", 1); r1.setAttribute("sample", "S-100"); r1.setAttribute("riskLevel", "low");
    int a = engine.startWorkflow(lab, r1);
    mediator.requestValidation(a);
    show(*engine.getInstance(a));
    std::cout << "  researcher may approve? " << engine.getInstance(a)->getApprovalPolicy()->canApprove(*engine.getInstance(a), researcher) << "\n";
    std::cout << "  head may approve?       " << engine.getInstance(a)->getApprovalPolicy()->canApprove(*engine.getInstance(a), head) << "\n";
    mediator.approvalCompleted(a, true);
    show(*engine.getInstance(a));
    engine.getInstance(a)->getRoot()->execute(1);

    std::cout << "\n== LAB: exception path (missing attribute) ==\n";
    WorkflowRequest r2("Unsafe reagent", 1); r2.setAttribute("sample", "S-101");
    int b = engine.startWorkflow(lab, r2);
    mediator.requestValidation(b);
    show(*engine.getInstance(b));

    // ---- Industry 2: loan application (auto-approval family), SAME engine/mediator ----
    std::shared_ptr<WorkflowDefinition> loan(new WorkflowDefinition(
        "Loan application", std::shared_ptr<WorkflowFactory>(new AutoApprovalComponentFactory()), "credit officer", 24));
    std::vector<std::string> loanKeys(1, "amount");
    std::shared_ptr<ValidationHandler> h(new RequiredAttributesHandler(loanKeys));
    h->setNext(std::shared_ptr<ValidationHandler>(new ThresholdHandler("amount", 500000)));
    loan->setValidationChain(h);
    std::vector<std::string> checks; checks.push_back("Credit check"); checks.push_back("Income check");
    loan->addStage("Assessment", checks);

    std::cout << "\n== LOAN: timeout auto-approval ==\n";
    WorkflowRequest r3("Home loan", 3); r3.setAttribute("amount", "250000");
    int c = engine.startWorkflow(loan, r3);
    mediator.requestValidation(c);
    show(*engine.getInstance(c));
    engine.getInstance(c)->advanceHours(24);
    am.checkTimeouts();
    show(*engine.getInstance(c));
    engine.getInstance(c)->getRoot()->execute(3);

    std::cout << "\n== LOAN: exception path (over limit) ==\n";
    WorkflowRequest r4("Mansion loan", 3); r4.setAttribute("amount", "900000");
    int d = engine.startWorkflow(loan, r4);
    mediator.requestValidation(d);
    show(*engine.getInstance(d));

    std::cout << "\n== history of lab instance " << a << " ==\n";
    const std::vector<std::string>& hist = engine.getInstance(a)->getHistory();
    for (size_t k = 0; k < hist.size(); ++k) std::cout << "  " << hist[k] << "\n";
    return 0;
}
