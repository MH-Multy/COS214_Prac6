#ifndef MEDIATOR_H
#define MEDIATOR_H
#include <iostream>
#include <string>
#include <vector>
#include "WorkItem.h"

class WorkflowMediator;

/** Colleague: knows only the mediator, never another colleague. */
class WorkflowColleague {
public:
    WorkflowColleague() : mediator(0) {}
    virtual ~WorkflowColleague() {}
    void setMediator(WorkflowMediator* m) { mediator = m; }
protected:
    WorkflowMediator* mediator;
};

/** Mediator interface: one named method per interaction (no switch on strings). */
class WorkflowMediator {
public:
    virtual ~WorkflowMediator() {}
    virtual void workSubmitted(WorkItem* item) = 0;
    virtual void approvalGranted(WorkItem* item) = 0;
    virtual void approvalRejected(WorkItem* item) = 0;
    virtual void timerExpired(WorkItem* item) = 0;
};

/* ---------------- Concrete colleagues ---------------- */
class TaskDispatcher : public WorkflowColleague {
public:
    void submit(WorkItem* item);                 // defined after mediator is known
    void holdQueue(WorkItem* i)   { std::cout << "  [Dispatcher] queue halted for '" << i->name << "'\n"; }
    void setRouter(const TaskRouter* r) { router = r; }
    TaskDispatcher() : router(0) {}
    void resume(WorkItem* i)      { i->status = WorkItem::RUNNING;
        std::cout << "  [Dispatcher] '" << i->name << "' running, assigned to "
                  << (router ? router->assign(*i) : "nobody") << "\n"; }
    void finish(WorkItem* i)      { i->status = WorkItem::COMPLETED;
                                    std::cout << "  [Dispatcher] '" << i->name << "' completed\n"; }
    void close(WorkItem* i)       { i->status = WorkItem::REJECTED;
                                    std::cout << "  [Dispatcher] '" << i->name << "' closed (rejected)\n"; }
private:
    const TaskRouter* router;
};

class ApprovalManager : public WorkflowColleague {
public:
    void createRequest(WorkItem* i) { i->status = WorkItem::WAITING_APPROVAL;
        std::cout << "  [Approval] request opened for '" << i->name << "'\n"; }
    void autoApprove(WorkItem* i)   { i->status = WorkItem::APPROVED;
        std::cout << "  [Approval] no reply in time -> auto-approved '" << i->name << "'\n"; }
    void approve(WorkItem* i);                   // human approves
    void reject(WorkItem* i);                    // human rejects
};

class ResourceAllocator : public WorkflowColleague {
public:
    void reserve(WorkItem* i) { std::cout << "  [Resources] reserved: " << i->resourceNeeded() << "\n"; }
    void release(WorkItem* i) { std::cout << "  [Resources] released: " << i->resourceNeeded() << "\n"; }
};

class EscalationTimer : public WorkflowColleague {
public:
    EscalationTimer() : item(0), remaining(0) {}
    void start(WorkItem* i, int ticks) { item = i; remaining = ticks;
        std::cout << "  [Timer] started (" << ticks << " ticks)\n"; }
    void cancel() { item = 0; }
    void tick();                                  // called by the engine's clock
private:
    WorkItem* item; int remaining;
};

/* ---------------- Concrete mediator ---------------- */
class WorkflowExecutionCoordinator : public WorkflowMediator {
public:
    WorkflowExecutionCoordinator(TaskDispatcher& d, ApprovalManager& a,
                                 ResourceAllocator& r, EscalationTimer& t)
        : disp(d), appr(a), res(r), timer(t) {
        d.setMediator(this); a.setMediator(this);
        r.setMediator(this); t.setMediator(this);
    }
    void workSubmitted(WorkItem* i) {
        if (!i->requiresApproval()) { res.reserve(i); disp.resume(i); disp.finish(i); res.release(i); return; }
        disp.holdQueue(i);
        appr.createRequest(i);
        timer.start(i, 3);
    }
    void approvalGranted(WorkItem* i) { timer.cancel(); res.reserve(i); disp.resume(i); }
    void approvalRejected(WorkItem* i) { timer.cancel(); disp.close(i); }   // exception path
    void timerExpired(WorkItem* i) { appr.autoApprove(i); approvalGranted(i); }
    void complete(WorkItem* i) { disp.finish(i); res.release(i); }
private:
    TaskDispatcher& disp; ApprovalManager& appr;
    ResourceAllocator& res; EscalationTimer& timer;
};

/* colleague methods that call the mediator */
inline void TaskDispatcher::submit(WorkItem* i) { mediator->workSubmitted(i); }
inline void ApprovalManager::approve(WorkItem* i) { i->status = WorkItem::APPROVED; mediator->approvalGranted(i); }
inline void ApprovalManager::reject(WorkItem* i)  { mediator->approvalRejected(i); }
inline void EscalationTimer::tick() {
    if (item && --remaining <= 0) { WorkItem* i = item; item = 0; mediator->timerExpired(i); }
}
#endif
