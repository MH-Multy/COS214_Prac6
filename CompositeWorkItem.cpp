#include "CompositeWorkItem.h"
#include "DepthFirstIterator.h"
#include "AvailableWorkIterator.h"

CompositeWorkItem::CompositeWorkItem(int id, string& name, bool sequential, const string& status):WorkItem(id, name, status){
    this->sequential = sequential;
}

bool CompositeWorkItem::execute( int actorId){
    bool executedAny = false;
    if(sequential){
        for(int i = 0; i < children.size(); i++){
            if(!children[i] -> isComplete()){
                executedAny = children[i]->execute(actorId);
                break;
            }
        }
    }
    else{
        for(int i = 0; i < children.size(); i++){
            if(children[i]-> isAvailable() && children[i]->execute(actorId)){
                executedAny = true;
            }
        }
    }
    if(isComplete()){
        status = "Completed";
    }
    return executedAny;
}

void CompositeWorkItem::add(shared_ptr<WorkItem> item){
    if(item){
        children.push_back(item);
    }
}

const vector<shared_ptr<WorkItem>>& CompositeWorkItem::getChildren(){
    return children;

}

bool CompositeWorkItem::isComplete(){
    for(int i = 0; i < children.size(); i++){
        if(!children[i] -> isComplete()){
            return false;
        }
    }
    return true;
}

unique_ptr<WorkItemIterator> CompositeWorkItem::createIterator(){
    return unique_ptr<WorkItemIterator>(new DepthFirstIterator(*this));
}


unique_ptr<WorkItemIterator> CompositeWorkItem::createAvailableIterator(){
    return unique_ptr<WorkItemIterator>(new AvailableWorkIterator(*this));

}

bool CompositeWorkItem::isAvailable(){
    //need to happen one after the other
    if(sequential){
        for(int i = 0; i < children.size(); i++){
            if(!children[i] -> isComplete()){
                return children[i]-> isAvailable();
            }
        }
        return false;
    }
    for(int i = 0; i < children.size(); i++){
        if(children[i]-> isAvailable()){
            return true;
        }
    }
    return false;
}