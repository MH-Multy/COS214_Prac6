#include "Task.h"

Task::Task(int id, const string& name, const string& requiredRole, const string& status):WorkItem(id, name, status){
    this -> assignedTo = -1;
    this -> requiredRole = requiredRole;
}

bool Task::execute( int actorId){
    if(!isAvailable() || actorId != assignedTo){
        return false;
    }
    status = "Completed";
    return true;
}

void Task::assignTo(int participantId){
    assignedTo = participantId;
}

int Task::getAssignee() const{
    return assignedTo;
}

string Task::getRequiredRole() const{
    return requiredRole;
}

bool Task::isComplete(){
    if(status == "Completed"){
        return true;
    }
    else{
        return false;
    }
}

bool Task::isAvailable(){
    if(status == "Approved"){
        return true;
    }
    else{
        return false;
    }
}
void Task::release(){
    if(status == "Pending"){
        status = "Approved";
    }
}

bool Task::reopen(){
    if(status != "Completed"){
        return false;
    }
    status = "Approved";
    return true;
}

const vector<shared_ptr<WorkItem>>& Task::getChildren() const{
    static const vector<shared_ptr<WorkItem>> none;
    return none;
}
