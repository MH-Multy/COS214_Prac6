#include "ReportHistory.h"
#include <stdexcept>
#include <string>

void ReportHistory::addSnapshot(const ReportMemento& m) 
{
    snapshots_.push_back(m);
}

const ReportMemento& ReportHistory::getSnapshot(int index) const 
{
    if (index < 0 || index >= static_cast<int>(snapshots_.size()))
        throw std::out_of_range("No report revision at position " + std::to_string(index));
    return snapshots_[static_cast<std::size_t>(index)];
}

int ReportHistory::size() const 
{
    return static_cast<int>(snapshots_.size());
}
