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
        Task(int id, string& name, string& requiredRole, const string& status = "Pending");
        bool execute( int actorId) override;
        void assignTo(int participantId);
        int getAssignee();
        string getRequiredRole();
        bool isComplete() override;
        bool isAvailable() override;
};


#endif //TASK_H