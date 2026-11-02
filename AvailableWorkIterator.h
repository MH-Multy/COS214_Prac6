/**
 * @file AvailableWorkIterator.h
 * @brief Concrete Iterator visiting only the work that can be done right now.
 */
#ifndef AVAILABLEWORKITERATOR_H
#define AVAILABLEWORKITERATOR_H

#include "DepthFirstIterator.h"

/**
 * @brief Visits only work a participant can act on right now: leaf items
 *        that are available and not yet complete.
 * @ingroup iterator
 *
 * Wraps a DepthFirstIterator and skips everything else, so the same tree
 * gives a different view without CompositeWorkItem changing.
 */
class AvailableWorkIterator : public WorkItemIterator{
    private:
        DepthFirstIterator inner;  ///< Walks the whole tree; this class filters it.
        /** @brief Advance inner until it points at an available, unfinished leaf, or is done. */
        void skipUnavailable();
    public:
        /** @param root Item whose available descendants are visited. */
        explicit AvailableWorkIterator(const WorkItem& root);
        /** @copydoc WorkItemIterator::first */
        void first() override;
        /** @copydoc WorkItemIterator::next */
        void next() override;
        /** @copydoc WorkItemIterator::isDone */
        bool isDone() const override;
        /** @copydoc WorkItemIterator::currentItem */
        shared_ptr<WorkItem> currentItem() const override;
};

#endif //AVAILABLEWORKITERATOR_H
