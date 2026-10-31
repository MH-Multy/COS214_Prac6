#include "WorkItem.h"

WorkItem::WorkItem(int id, const string& name, const string& status){
    this->id = id;
    this -> name = name;
    this -> status = status;
}

WorkItem::~WorkItem(){}

int WorkItem::getId(){
    return id;
}

string WorkItem::getName(){
    return name;
}

string WorkItem::getStatus(){
    return status;
}


bool WorkItem::reopen(){
    return false;
}

void WorkItem::assignTo(int){
}

int WorkItem::getAssignee() const{
    return -1;
}

string WorkItem::getRequiredRole() const{
    return "";
}
