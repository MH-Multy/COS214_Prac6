#ifndef REPORTHISTORY_H
#define REPORTHISTORY_H

#include <vector>
#include "ReportMemento.h"

/**
 *  Memento Caretaker: keeps every saved revision of one workflow's
 *        report so a participant can roll back to any earlier version.
 *
 * The caretaker stores and returns snapshots but never looks inside them.
 * Each WorkflowInstance owns its own ReportHistory, so report revisions of
 * one instance can never affect another instance.
 */
class ReportHistory 
{
public:
    void addSnapshot(const ReportMemento& m);

    /**
     *  Snapshot at position  index (0 is the oldest).
     * @throws std::out_of_range if  index is not a valid position.
     */
    const ReportMemento& getSnapshot(int index) const;

    int size() const;

private:
    std::vector<ReportMemento> snapshots_;
};

#endif
