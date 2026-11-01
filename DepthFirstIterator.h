/**
 * @file DepthFirstIterator.h
 * @brief Concrete Iterator visiting every work item below a root, depth first.
 */
#ifndef DEPTHFIRSTITERATOR_H
#define DEPTHFIRSTITERATOR_H

#include <vector>
#include "WorkItemIterator.h"

/**
 * @brief Visits every work item below a root, depth first (a stage, then
 *        its tasks, then the next stage...). The root itself is not visited.
 * @ingroup iterator
 *
 * The tree is flattened once, on construction, by recursively calling
 * getChildren(). Every WorkItem answers getChildren() (a Task returns an
 * empty list), so no type checks are needed.
 */
class DepthFirstIterator : public WorkItemIterator{
    private:
        vector<shared_ptr<WorkItem>> items;  ///< The tree, flattened in visiting order.
        size_t index;                        ///< Current position in items.
        /** @brief Append @p item's descendants to items, depth first. */
        void collect(const WorkItem& item);
    public:
        /** @param root Item whose descendants are visited; usually CompositeWorkItem::createIterator() passes itself. */
        explicit DepthFirstIterator(const WorkItem& root);
        /** @copydoc WorkItemIterator::first */
        void first() override;
        /** @copydoc WorkItemIterator::next */
        void next() override;
        /** @copydoc WorkItemIterator::isDone */
        bool isDone() const override;
        /** @copydoc WorkItemIterator::currentItem */
        shared_ptr<WorkItem> currentItem() const override;
};

#endif //DEPTHFIRSTITERATOR_H
