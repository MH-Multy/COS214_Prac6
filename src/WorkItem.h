#ifndef WORKITEM_H
#define WORKITEM_H
#include <string>

/** ABSTRACT PRODUCT A. STAND-IN for your Composite WorkItem: keep your real
 *  class and just make sure it has these virtuals. */
class WorkItem {
public:
    enum Status { CREATED, WAITING_APPROVAL, APPROVED, RUNNING, COMPLETED, REJECTED };
    WorkItem(const std::string& n) : name(n), status(CREATED) {}
    virtual ~WorkItem() {}
    virtual bool requiresApproval() const = 0;
    virtual std::string resourceNeeded() const = 0;
    virtual std::string industry() const = 0;
    std::string name;
    Status status;
};

/** ABSTRACT PRODUCT B: the data carried by one workflow instance. */
class Payload {
public:
    virtual ~Payload() {}
    virtual std::string describe() const = 0;
};

/** ABSTRACT PRODUCT C: decides who does the work (your Strategy/TaskRouter). */
class TaskRouter {
public:
    virtual ~TaskRouter() {}
    virtual std::string assign(const WorkItem& item) const = 0;
};
#endif
