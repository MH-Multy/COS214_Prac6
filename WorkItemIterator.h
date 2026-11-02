/**
 * @file WorkItemIterator.h
 * @brief Iterator interface for walking a tree of work items.
 */
#ifndef WORKITEMITERATOR_H
#define WORKITEMITERATOR_H

#include <memory>
#include "WorkItem.h"

using namespace std;

/**
 * @brief Iterator interface for walking a tree of work items.
 * @ingroup iterator
 *
 * Clients (the workflow instance, the Facade, the TUI) traverse the work of
 * an instance through this interface, so they never depend on how a
 * CompositeWorkItem stores its children.
 */
class WorkItemIterator{
    public:
        /** @brief Virtual destructor so iterators can be deleted through a base pointer. */
        virtual ~WorkItemIterator(){}
        /** @brief Move to the first item (restarts the traversal). */
        virtual void first() = 0;
        /** @brief Move to the next item; does nothing once done. */
        virtual void next() = 0;
        /** @brief Whether the traversal has passed the last item. @return true when there are no more items. */
        virtual bool isDone() const = 0;
        /** @brief The item at the current position. @return The item, or an empty pointer once the traversal is done. */
        virtual shared_ptr<WorkItem> currentItem() const = 0;
};

#endif //WORKITEMITERATOR_H
