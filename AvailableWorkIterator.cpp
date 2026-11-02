/**
 * @file AvailableWorkIterator.cpp
 * @brief Implementation of AvailableWorkIterator.
 */
#include "AvailableWorkIterator.h"

AvailableWorkIterator::AvailableWorkIterator(const WorkItem& root) : inner(root){
    skipUnavailable();
}

void AvailableWorkIterator::skipUnavailable(){
    while(!inner.isDone()){
        shared_ptr<WorkItem> item = inner.currentItem();
        // Only leaves are actionable work; a stage is "available" only because one of
        // its tasks is, and that task is visited on its own.
        bool isLeaf = item->getChildren().empty();
        if(isLeaf && item->isAvailable() && !item->isComplete()){
            return;
        }
        inner.next();
    }
}

void AvailableWorkIterator::first(){
    inner.first();
    skipUnavailable();
}

void AvailableWorkIterator::next(){
    inner.next();
    skipUnavailable();
}

bool AvailableWorkIterator::isDone() const{
    return inner.isDone();
}

shared_ptr<WorkItem> AvailableWorkIterator::currentItem() const{
    return inner.currentItem();
}
