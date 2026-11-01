/**
 * @file RequestSummaryBuilder.cpp
 * @brief Implementation of RequestSummaryBuilder.
 */
#include "RequestSummaryBuilder.h"

void RequestSummaryBuilder::reset(){
    summary.str("");
    summary.clear();
}

void RequestSummaryBuilder::buildHeader(const string& name, int requesterId){
    summary << "Request: " << name << " (requester " << requesterId << ")\n";
}

void RequestSummaryBuilder::buildAttribute(const string& key, const string& value){
    summary << "  " << key << ": " << value << "\n";
}

void RequestSummaryBuilder::buildDeadline(int days){
    summary << "  Deadline: " << days << " days\n";
}

void RequestSummaryBuilder::buildPriority(int level){
    summary << "  Priority: " << level << "\n";
}

string RequestSummaryBuilder::getResult(){
    return summary.str();
}