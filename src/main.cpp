#include "WorkflowEngine.h"

int main() {
    TaskDispatcher d; ApprovalManager a; ResourceAllocator r; EscalationTimer t;
    WorkflowExecutionCoordinator co(d, a, r, t);
    LabTestingComponentFactory labF; LoanComponentFactory loanF;

    { WorkflowEngine e(labF, d);                       // happy path
      std::unique_ptr<WorkItem> i = e.start("Toxicology test");
      a.approve(i.get()); co.complete(i.get()); }
    { WorkflowEngine e(labF, d);                       // exception path
      std::unique_ptr<WorkItem> i = e.start("Unsafe reagent");
      a.reject(i.get()); }
    { WorkflowEngine e(loanF, d);                      // second industry + timeout
      std::unique_ptr<WorkItem> i = e.start("Home loan #42");
      for (int k = 0; k < 3; ++k) t.tick();
      co.complete(i.get()); }
    return 0;
}
