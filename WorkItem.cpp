#include "WorkItem.h"

WorkItem::WorkItem(int id, string& name, const string& status){
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

