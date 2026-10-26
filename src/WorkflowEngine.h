#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H
#include "ComponentFactory.h"
#include "Mediator.h"

/** Generic engine: the Abstract Factory's client. Contains no lab/loan code. */
class WorkflowEngine {
public:
    WorkflowEngine(const WorkflowComponentFactory& f, TaskDispatcher& d)
        : factory(f), disp(d) {}
    /** Starts one instance; returns the item so the caller can act on it. */
    std::unique_ptr<WorkItem> start(const std::string& name) {
        std::unique_ptr<WorkItem> item = factory.createWorkItem(name);
        payload = factory.createPayload();
        router  = factory.createRouter();
        disp.setRouter(router.get());
        std::cout << "\n== " << item->industry() << ": " << name
                  << " | payload " << payload->describe() << " ==\n";
        disp.submit(item.get());                // Mediator takes over
        return item;
    }
private:
    const WorkflowComponentFactory& factory;
    TaskDispatcher& disp;
    std::unique_ptr<Payload> payload;
    std::unique_ptr<TaskRouter> router;
};
#endif
