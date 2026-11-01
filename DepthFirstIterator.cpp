/**
 * @file DepthFirstIterator.cpp
 * @brief Implementation of DepthFirstIterator.
 */
#include "DepthFirstIterator.h"

DepthFirstIterator::DepthFirstIterator(const WorkItem& root) : index(0){
    collect(root);
}

void DepthFirstIterator::collect(const WorkItem& item){
    const vector<shared_ptr<WorkItem>>& children = item.getChildren();
    for(size_t i = 0; i < children.size(); i++){
        items.push_back(children[i]);
        collect(*children[i]);
    }
}

void DepthFirstIterator::first(){
    index = 0;
}

void DepthFirstIterator::next(){
    if(!isDone()){
        index++;
    }
}

bool DepthFirstIterator::isDone() const{
    return index >= items.size();
}

shared_ptr<WorkItem> DepthFirstIterator::currentItem() const{
    if(isDone()){
        return shared_ptr<WorkItem>();
    }
    return items[index];
}
