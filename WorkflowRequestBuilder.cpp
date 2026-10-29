#include "WorkflowRequestBuilder.h"

void WorkflowRequestBuilder::reset(){
    request = WorkflowRequest();
}

void WorkflowRequestBuilder::buildHeader(const string& name, int requesterId){
    request.name = name;
    request.requesterId = requesterId;
}

void WorkflowRequestBuilder::buildAttribute(const string& key, const string& value){
    request.setAttribute(key, value);
}

void WorkflowRequestBuilder::buildDeadline(int days){
    request.deadlineDays = days;
}

void WorkflowRequestBuilder::buildPriority(int level){
    request.priority = level;
}

WorkflowRequest WorkflowRequestBuilder::getResult(){
    return request;
}