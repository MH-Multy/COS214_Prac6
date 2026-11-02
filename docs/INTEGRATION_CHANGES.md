# Integration changes (from the GitHub `dev` branch)

What was changed to make the system build, run and match class diagram v5, and why.

## 1. Merge conflicts resolved
| Conflict | Resolution |
|---|---|
| `ReportHistory`, `ReportMemento`, `WorkflowDefinition`, `WorkflowEvent`, `WorkflowReport` existed at the root **and** in `include/`/`src/` (identical copies) | Kept one copy (root). |
| `WorkflowInstance` in **three** versions: root (with Observer), `include/src` (without), lowercase `workflowInstance.h/.cpp` (State stub). On Windows/macOS `workflowInstance.h` and `WorkflowInstance.h` are the same file. | One `WorkflowInstance`, implemented per v5 (keeps the Observer version's `WorkflowSubject` base). Lowercase copies removed. |
| `WorkflowRequest.h` contained **two** class definitions (both sides of a merge pasted together) | Kept Zander's Builder product; added the `(name, requesterId)` constructor the Command code used from the other version. |
| Files split between the root and `include/` + `src/` | Everything at the root, matching the team's Makefile (`$(wildcard *.cpp)`). |
| `test_runner` (a compiled binary) was committed | Removed; `run_tests` added to `.gitignore`. |

## 2. Errors fixed in existing code (minimum needed to compile/work)
- **State (`workflowState.h/.cpp`)**: `illegalState` received a string literal for `string&`; `const` transitions called a non-`const` function; `DeniedSate` typo; `WorkflowInstance` never included. Fixed. `illegalState` now records an `ILLEGAL_STATE` event instead of printing. `SuspendedState::accepted()` (from v5) implemented as **resume**.
- **Strategy (`ApprovalPolicy`)**: `.cpp` used `WorkflowInstance` without including it. Added v5's `approverRole` and `canApprove()`; `shouldAutoApprove()` logic unchanged.
- **Composite (`WorkItem`, `Task`, `CompositeWorkItem`)**: `Task` never implemented `getChildren()` (it was abstract); `string&` parameters rejected text; `getChildren()` made `const` (needed by v5's `DepthFirstIterator(const WorkItem&)`). Added `release()` (tasks could never become available), `reopen()` (undo), and `assignTo/getAssignee/getRequiredRole` on `WorkItem` (routing without type checks).
- **Builder**: `WorkflowRequestDirector` crashed with no builder; now throws `invalid_argument`.
- **Makefile**: `-std=c++20` → `-std=c++11` (spec requirement); added `test` and `demo` targets.
- **`EditReportCommand`**: the report is locked while suspended or finished (scenario: a suspended researcher cannot keep logs). All other Command classes are **unchanged**.

## 3. v5 classes that were missing and are now implemented
Iterator (`WorkItemIterator`, `DepthFirstIterator`, `AvailableWorkIterator`), Abstract Factory
(`WorkflowFactory`, `AutoApprovalComponentFactory`, `ManualReviewComponentFactory`), Mediator
(`DefaultWorkflowMediator`, `WorkflowColleague`, `ValidationService`, `ApprovalManager`,
`NotificationService`), `Participant`, the real `WorkflowEngine`, `WorkflowDefinition`,
`WorkflowInstance` and `WorkflowFacade`, plus `main.cpp`, `TerminalUI` (the TUI) and `IndustrySetup`.

Unchanged team code: Observer, Chain of Responsibility, Adapter, Memento, all Command classes except
`EditReportCommand`, the Builder classes, `Dockerfile`, `docker-compose.yml`, `Doxyfile`.

## 4. Visual Paradigm: small edits so v5 matches the code
- Add class **TerminalUI** (uses `WorkflowFacade`) and **EditReportCommand** (a ConcreteCommand).
- `WorkItemState::completed(workflow : WorkflowInstance&)` (v5 shows `item : WorkItem&`).
- `WorkflowInstance`: `suspend(actorId : int)`, `resume(actorId : int)`; `setState(next : WorkItemState*)`;
  add `reopenItem`, `findItem`, `getHistory`, `getReport`, `getReportHistory`, `getRoot`, `getAcceptedAt`.
- `WorkItem`: add `release()`, `reopen()`, `assignTo()`, `getAssignee()`, `getRequiredRole()`.
- `WorkflowDefinition`: add `approverRole`, `description`, `requestFields`, `sequentialStages` and setters.
  Tasks inside `stages` are written as `"Task name (Role)"`.
- `CommandInvoker::undoLast(workflowId : int)` and `WorkflowFacade::undoLastAction(workflowId : int)`.
- `WorkflowFacade`: add `getWorkflow`, `getDefinition`, `canApprove`, `processTimeouts`, report operations,
  participant operations, `getNotifications`, `getAuditLog`, `getApproverQueue`, `getActionHistory`.
- `WorkflowMediator`: add `processTimeouts()`, `getNotifications()`. `ApprovalManager`: attribute `engine`.
  `NotificationService`: `getMessages()`. `TimeoutApprovalPolicy.timeoutSeconds` (not hours);
  `StrictApprovalPolicy` applies the four-eyes rule (no `seniorRole`). `ApprovalPolicy::describe()`.
- `WorkflowEngine`: add `listParticipants`, `addGlobalObserver`, `getRequesterInbox`.
