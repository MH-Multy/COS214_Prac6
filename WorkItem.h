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
        WorkItem(int id, string& name, const string& status);
        virtual ~WorkItem();
        virtual bool execute(int actorId) = 0;
        int getId();
        string getName();
        string getStatus();
        virtual bool isAvailable() =0;
        virtual bool isComplete() = 0;
        virtual const vector<shared_ptr<WorkItem>>& getChildren() = 0;

};


#endif //WORKITEM_H