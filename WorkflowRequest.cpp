#include "WorkflowRequest.h"

WorkflowRequest::WorkflowRequest(){
    this -> name = "";
    this -> requesterId = 0;
    this -> deadlineDays = 0;
    this -> priority = 0;
}

string WorkflowRequest::getName() const{
    return name;
}

int WorkflowRequest::getRequesterId() const{
    return requesterId;
}

string WorkflowRequest::getAttribute(const string& key) const{
    map<string, string>::const_iterator it = attributes.find(key);
    if(it == attributes.end()){
        return "";
    }
    return it->second;
}

void WorkflowRequest::setAttribute(const string& key, const string& value){
    attributes[key] = value;
}

int WorkflowRequest::getDeadlineDays() const{
    return deadlineDays;
}

int WorkflowRequest::getPriority() const{
    return priority;
}