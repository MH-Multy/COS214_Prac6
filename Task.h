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
        bool execute( int actorId);
        void assignTo(int participantId);
        int getAssignee();
        string getRequiredRole();
};


#endif //TASK_H