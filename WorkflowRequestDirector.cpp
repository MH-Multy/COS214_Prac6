#include "WorkflowRequestDirector.h"

WorkflowRequestDirector::WorkflowRequestDirector(WorkflowBuilder* b) : builder(b){
}

void WorkflowRequestDirector::setBuilder(WorkflowBuilder* b){
    builder = b;
}

void WorkflowRequestDirector::constructStandard(const map<string, string>& input){
    string name;
    int requesterId = 0;

    map<string, string>::const_iterator it = input.find("name");
    if(it != input.end()){
        name = it->second;
    }
    it = input.find("requesterId");
    if(it != input.end()){
        requesterId = stoi(it->second);
    }

    builder->reset();
    builder->buildHeader(name, requesterId);
    for(it = input.begin(); it != input.end(); ++it){
        if(it->first != "name" && it->first != "requesterId"){
            builder->buildAttribute(it->first, it->second);
        }
    }
    builder->buildDeadline(14);
    builder->buildPriority(1);
}

void WorkflowRequestDirector::constructUrgent(const map<string, string>& input){
    string name;
    int requesterId = 0;

    map<string, string>::const_iterator it = input.find("name");
    if(it != input.end()){
        name = it->second;
    }
    it = input.find("requesterId");
    if(it != input.end()){
        requesterId = stoi(it->second);
    }

    builder->reset();
    builder->buildHeader(name, requesterId);
    for(it = input.begin(); it != input.end(); ++it){
        if(it->first != "name" && it->first != "requesterId"){
            builder->buildAttribute(it->first, it->second);
        }
    }
    builder->buildDeadline(2);
    builder->buildPriority(5);
}