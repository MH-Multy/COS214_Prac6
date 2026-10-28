#ifndef COMPOSITEWORKITEM_H
#define COMPOSITEWORKITEM_H

#include "WorkItem.h"
#include <string>
#include <memory>

class WorkItemIterator;


class CompositeWorkItem : public WorkItem{
    private:
        vector<shared_ptr<WorkItem>> children;
        bool sequential;
    public:
        CompositeWorkItem(int id, string& name, bool sequential = false, const string& status="Pending");
        bool execute( int actorId) override;
        void add(shared_ptr<WorkItem> item);
        const vector<shared_ptr<WorkItem>>& getChildren();
        bool isComplete() override;
        unique_ptr<WorkItemIterator> createIterator();
        unique_ptr<WorkItemIterator> createAvailableIterator();
        bool isAvailable() override;
};

#endif //COMPOSITEWORKITEM_H