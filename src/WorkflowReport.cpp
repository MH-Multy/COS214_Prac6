#include "WorkflowReport.h"

WorkflowReport::WorkflowReport() : content_(""), revision_(0) {}

void WorkflowReport::edit(const std::string& text) 
{
    content_ = text;
    ++revision_;
}

ReportMemento WorkflowReport::save() const 
{
    return ReportMemento(content_, revision_);
}

void WorkflowReport::restore(const ReportMemento& m) {
    content_ = m.content_;
    revision_ = m.revision_;
}

std::string WorkflowReport::getContent() const { return content_; }

int WorkflowReport::getRevision() const { return revision_; }

std::string WorkflowReport::preview(const ReportMemento& m, std::size_t maxLength) 
{
    std::string text = m.content_;
    for (std::size_t i = 0; i < text.size(); ++i)
        if (text[i] == '\n') text[i] = ' ';
    if (text.size() > maxLength) text = text.substr(0, maxLength) + "...";
    return text;
}
