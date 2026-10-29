#ifndef STANDINS_H
#define STANDINS_H
/**
 * STAND-INS for classes owned by teammates (names/signatures follow
 * TIVIDY_ClassDiagram_v4). They exist ONLY so the Mediator and Abstract
 * Factory compile and run on their own. When merging, delete the section
 * whose owner has pushed the real class.
 */
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class WorkflowInstance;
class WorkflowDefinition;
class WorkflowFactory;

/* ---------- core value types ---------- */
class ValidationResult {
public:
    ValidationResult(bool p = true, const std::string& r = "") : passed(p), reason(r) {}
    bool isPassed() const { return passed; }
    std::string getReason() const { return reason; }
private:
    bool passed; std::string reason;
};

class Participant {
public:
    Participant(int i = 0, const std::string& n = "",
                const std::vector<std::string>& r = std::vector<std::string>())
        : id(i), name(n), roles(r) {}
    int getId() const { return id; }
    std::string getName() const { return name; }
    bool hasRole(const std::string& role) const {
        for (size_t k = 0; k < roles.size(); ++k) if (roles[k] == role) return true;
        return false;
    }
private:
    int id; std::string name; std::vector<std::string> roles;
};

class WorkflowRequest {
public:
    WorkflowRequest(const std::string& n = "", int requester = 0) : name(n), requesterId(requester) {}
    std::string getName() const { return name; }
    int getRequesterId() const { return requesterId; }
    void setAttribute(const std::string& k, const std::string& v) { attributes[k] = v; }
    std::string getAttribute(const std::string& k) const {
        std::map<std::string, std::string>::const_iterator it = attributes.find(k);
        return it == attributes.end() ? std::string() : it->second;
    }
private:
    std::string name; int requesterId; std::map<std::string, std::string> attributes;
};

/* ---------- Composite (Zander): WorkItem / Task / CompositeWorkItem ---------- */
class WorkItem {
public:
    WorkItem(const std::string& n) : id(nextId()), name(n), status("pending") {}
    virtual ~WorkItem() {}
    virtual bool execute(int actorId) = 0;
    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getStatus() const { return status; }
    virtual bool isComplete() const { return status == "done"; }
protected:
    int id; std::string name; std::string status;
private:
    static int nextId() { static int c = 1; return c++; }
};

class Task : public WorkItem {
public:
    Task(const std::string& n) : WorkItem(n) {}
    bool execute(int actorId) {
        status = "done";
        std::cout << "      task '" << name << "' done by participant " << actorId << "\n";
        return true;
    }
};

class CompositeWorkItem : public WorkItem {
public:
    CompositeWorkItem(const std::string& n, bool seq) : WorkItem(n), sequential(seq) {}
    void add(std::shared_ptr<WorkItem> item) { children.push_back(item); }
    const std::vector<std::shared_ptr<WorkItem> >& getChildren() const { return children; }
    bool isSequential() const { return sequential; }
    bool execute(int actorId) {
        std::cout << "    group '" << name << "' (" << (sequential ? "sequential" : "parallel") << ")\n";
        bool ok = true;
        for (size_t k = 0; k < children.size(); ++k) ok = children[k]->execute(actorId) && ok;
        status = "done";
        return ok;
    }
    bool isComplete() const {
        for (size_t k = 0; k < children.size(); ++k) if (!children[k]->isComplete()) return false;
        return true;
    }
private:
    std::vector<std::shared_ptr<WorkItem> > children; bool sequential;
};

/* ---------- Strategy (Mohammad): ApprovalPolicy base ---------- */
class ApprovalPolicy {
public:
    ApprovalPolicy(const std::string& role) : approverRole(role) {}
    virtual ~ApprovalPolicy() {}
    virtual bool canApprove(const WorkflowInstance& wf, const Participant& actor) const = 0;
    virtual bool shouldAutoApprove(const WorkflowInstance& wf) const = 0;
    std::string getApproverRole() const { return approverRole; }
protected:
    std::string approverRole;
};

/* ---------- Chain of Responsibility (Nokhaya): ValidationHandler base ---------- */
class ValidationHandler {
public:
    virtual ~ValidationHandler() {}
    std::shared_ptr<ValidationHandler> setNext(std::shared_ptr<ValidationHandler> h) { next = h; return h; }
    ValidationResult handle(const WorkflowInstance& wf) const {
        ValidationResult r = check(wf);
        if (!r.isPassed() || !next) return r;
        return next->handle(wf);
    }
protected:
    virtual ValidationResult check(const WorkflowInstance& wf) const = 0;
private:
    std::shared_ptr<ValidationHandler> next;
};

/* ---------- Definition (core). Industry DATA lives here, never in the engine ---------- */
class WorkflowDefinition {
public:
    WorkflowDefinition(const std::string& n, std::shared_ptr<WorkflowFactory> f,
                       const std::string& role, int timeoutHours)
        : name(n), factory(f), approverRole(role), approvalTimeout(timeoutHours) {}
    void addStage(const std::string& stage, const std::vector<std::string>& tasks) {
        stages.push_back(std::make_pair(stage, tasks));
    }
    void setValidationChain(std::shared_ptr<ValidationHandler> h) { validationChain = h; }
    std::string getName() const { return name; }
    std::string getApproverRole() const { return approverRole; }
    int getApprovalTimeout() const { return approvalTimeout; }
    const std::vector<std::pair<std::string, std::vector<std::string> > >& getStages() const { return stages; }
    std::shared_ptr<ValidationHandler> getValidationChain() const { return validationChain; }
    std::shared_ptr<WorkflowFactory> getFactory() const { return factory; }
private:
    std::string name;
    std::shared_ptr<WorkflowFactory> factory;
    std::string approverRole; int approvalTimeout;
    std::vector<std::pair<std::string, std::vector<std::string> > > stages;
    std::shared_ptr<ValidationHandler> validationChain;
};

/* ---------- State (Mohammad): simplified stand-in for WorkflowInstance + states ---------- */
class WorkflowInstance {
public:
    WorkflowInstance(int i, std::shared_ptr<const WorkflowDefinition> d, const WorkflowRequest& r)
        : id(i), definition(d), request(r), state("PendingValidation"), waited(0) { addEvent("created"); }
    int getId() const { return id; }
    const WorkflowRequest& getRequest() const { return request; }
    std::shared_ptr<const WorkflowDefinition> getDefinition() const { return definition; }
    void setApprovalPolicy(std::shared_ptr<ApprovalPolicy> p) { policy = p; }
    std::shared_ptr<ApprovalPolicy> getApprovalPolicy() const { return policy; }
    void setRoot(std::shared_ptr<CompositeWorkItem> r) { root = r; }
    std::shared_ptr<CompositeWorkItem> getRoot() const { return root; }

    bool validate()            { return move("PendingValidation", "PendingApproval"); }
    bool approve(int /*actorId*/) { return move("PendingApproval", "Approved"); }
    bool reject(int /*actorId*/, const std::string& reason) {
        if (state == "PendingValidation" || state == "PendingApproval") {
            addEvent(state + " -> Rejected (" + reason + ")"); state = "Rejected"; return true;
        }
        addEvent("illegal reject in " + state); return false;
    }
    bool start()    { return move("Approved", "InProgress"); }
    bool complete() { return move("InProgress", "Completed"); }
    std::string getStateName() const { return state; }
    bool shouldAutoApprove() const { return policy && policy->shouldAutoApprove(*this); }
    void advanceHours(int h) { waited += h; }          // simulated clock
    int hoursWaiting() const { return waited; }
    void addEvent(const std::string& e) { history.push_back(e); }
    const std::vector<std::string>& getHistory() const { return history; }
private:
    bool move(const std::string& from, const std::string& to) {
        if (state != from) { addEvent("illegal transition to " + to + " from " + state); return false; }
        addEvent(from + " -> " + to); state = to; return true;
    }
    int id;
    std::shared_ptr<const WorkflowDefinition> definition;
    WorkflowRequest request;
    std::string state; int waited;
    std::shared_ptr<ApprovalPolicy> policy;
    std::shared_ptr<CompositeWorkItem> root;
    std::vector<std::string> history;
};

/* ---------- concrete Strategies (Mohammad) ---------- */
class TimeoutApprovalPolicy : public ApprovalPolicy {
public:
    TimeoutApprovalPolicy(const std::string& role, int hours) : ApprovalPolicy(role), timeoutHours(hours) {}
    bool canApprove(const WorkflowInstance&, const Participant& a) const { return a.hasRole(approverRole); }
    bool shouldAutoApprove(const WorkflowInstance& wf) const { return wf.hoursWaiting() >= timeoutHours; }
private:
    int timeoutHours;
};
class StrictApprovalPolicy : public ApprovalPolicy {
public:
    StrictApprovalPolicy(const std::string& seniorRole) : ApprovalPolicy(seniorRole) {}
    bool canApprove(const WorkflowInstance&, const Participant& a) const { return a.hasRole(approverRole); }
    bool shouldAutoApprove(const WorkflowInstance&) const { return false; }
};

/* ---------- concrete handlers (Nokhaya) ---------- */
class RequiredAttributesHandler : public ValidationHandler {
public:
    RequiredAttributesHandler(const std::vector<std::string>& k) : keys(k) {}
protected:
    ValidationResult check(const WorkflowInstance& wf) const {
        for (size_t i = 0; i < keys.size(); ++i)
            if (wf.getRequest().getAttribute(keys[i]).empty())
                return ValidationResult(false, "missing attribute '" + keys[i] + "'");
        return ValidationResult(true);
    }
private:
    std::vector<std::string> keys;
};
class ThresholdHandler : public ValidationHandler {
public:
    ThresholdHandler(const std::string& a, double m) : attribute(a), maxValue(m) {}
protected:
    ValidationResult check(const WorkflowInstance& wf) const {
        double v = std::atof(wf.getRequest().getAttribute(attribute).c_str());
        if (v > maxValue) return ValidationResult(false, attribute + " exceeds limit");
        return ValidationResult(true);
    }
private:
    std::string attribute; double maxValue;
};
#endif