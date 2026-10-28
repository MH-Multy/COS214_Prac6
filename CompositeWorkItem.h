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
        bool execute( int actorId);
        void add(shared_ptr<WorkItem> item);
        const vector<shared_ptr<WorkItem>>& getChildren();
        bool isComplete();
        unique_ptr<WorkItemIterator> createAvailableIterator();
};

#endif //COMPOSITEWORKITEM_H