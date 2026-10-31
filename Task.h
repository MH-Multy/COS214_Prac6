#ifndef TASK_H
#define TASK_H

#include <string>
#include "WorkItem.h"

using namespace std;

class Task: public WorkItem{
    private:
        int assignedTo;
        string requiredRole;
    public:
        Task(int id, const string& name, const string& requiredRole, const string& status = "Pending");
        bool execute( int actorId) override;
        void assignTo(int participantId) override;
        int getAssignee() const override;
        string getRequiredRole() const override;
        void release() override;                 // Pending -> Approved (available)
        bool reopen() override;                  // Completed -> Approved
        const vector<shared_ptr<WorkItem>>& getChildren() const override;   // a leaf: always empty
        bool isComplete() override;
        bool isAvailable() override;
};


#endif //TASK_H