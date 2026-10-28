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
        virtual bool execute(int actorId) = 0;
        int getId;
        string getName();
        string getStatus();
        bool isAvailable();
        bool isComplete();
        const vector<shared_ptr<WorkItem>>& getChildren();

};


#endif //WORKITEM_H