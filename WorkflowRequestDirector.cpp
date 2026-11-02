/**
 * @file WorkflowRequestDirector.cpp
 * @brief Implementation of WorkflowRequestDirector.
 */
#include "WorkflowRequestDirector.h"
#include <stdexcept>

WorkflowRequestDirector::WorkflowRequestDirector(WorkflowBuilder* b) : builder(b){
}

void WorkflowRequestDirector::setBuilder(WorkflowBuilder* b){
    builder = b;
}

void WorkflowRequestDirector::constructStandard(const map<string, string>& input){
    if(builder == nullptr){
        throw invalid_argument("WorkflowRequestDirector has no builder");
    }
    string name;
    int requesterId = 0;

    map<string, string>::const_iterator it = input.find("name");
    if(it != input.end()){
        name = it->second;
    }
    it = input.find("requesterId");
    if(it != input.end()){
        // stoi throws std::invalid_argument for non-numeric input; it is left to propagate
        // so the user interface can report the bad value.
        requesterId = stoi(it->second);
    }

    // Fixed construction order: the director decides the sequence, the builder decides the result.
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
    if(builder == nullptr){
        throw invalid_argument("WorkflowRequestDirector has no builder");
    }
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