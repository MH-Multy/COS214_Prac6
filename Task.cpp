#include "Task.h"

Task::Task(int id, string& name, string& requiredRole, const string& status):WorkItem(id, name, status){
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

int Task::getAssignee(){
    return assignedTo;
}

string Task::getRequiredRole(){
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