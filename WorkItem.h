#ifndef WORKITEM_H
#define WORKITEM_H

#include <string>
#include <vector>
#include <memory>

using namespace std;

class WorkItem{
    protected:
        int id;
        string name;
        string status;
    public:
        WorkItem(int id, const string& name, const string& status);
        virtual ~WorkItem();
        virtual bool execute(int actorId) = 0;
        int getId();
        string getName();
        string getStatus();
        virtual bool isAvailable() =0;
        virtual bool isComplete() = 0;
        virtual const vector<shared_ptr<WorkItem>>& getChildren() const = 0;

        // Added so work can be released, undone and routed without type checks:
        virtual void release() = 0;              // make the work that is now allowed available
        virtual bool reopen();                   // undo a completion (groups return false)
        virtual void assignTo(int participantId); // groups ignore this
        virtual int getAssignee() const;         // -1 for groups / unassigned
        virtual string getRequiredRole() const;  // "" for groups

};


#endif //WORKITEM_H