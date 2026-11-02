/**
 * @file TerminalUI.cpp
 * @brief The text menu. Each option asks a few questions, then calls the WorkflowFacade.
 */
#include "TerminalUI.h"
#include <cerrno>
#include <cstdlib>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "ApprovalPolicy.h"
#include "CompositeWorkItem.h"
#include "IndustrySetup.h"
#include "RequestSummaryBuilder.h"
#include "WorkItem.h"
#include "WorkflowDefinition.h"
#include "WorkflowFacade.h"
#include "WorkflowInstance.h"
#include "WorkflowRequestBuilder.h"
#include "WorkflowRequestDirector.h"
#if defined(__unix__) || defined(__APPLE__)
#include <poll.h>
#include <unistd.h>
#endif
using namespace std;

namespace {
// Thrown when there is no more input, so the program can end cleanly.
struct InputEnded {};
// Thrown when a request was auto-approved while a menu waited for typing, so the screen is drawn again.
struct ScreenChanged {};

string trim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}
// "a, b, c" -> {"a", "b", "c"}
vector<string> splitByComma(const string& text) {
    vector<string> parts;
    stringstream stream(text);
    string part;
    while (getline(stream, part, ','))
        if (!trim(part).empty()) parts.push_back(trim(part));
    return parts;
}

// ---- Look and feel (presentation only). Colour codes are added only when colour is on.
const int WIDTH = 76;   // width of the lines drawn on screen
string paint(bool colour, const string& code, const string& text) {
    return colour ? "\033[" + code + "m" + text + "\033[0m" : text;
}
string repeat(const string& piece, int times) {
    string line;
    for (int i = 0; i < times; ++i) line += piece;
    return line;
}
size_t visible(const string& text) {   // characters on screen (symbols like · take several bytes)
    size_t count = 0;
    for (size_t i = 0; i < text.size(); ++i) if ((text[i] & 0xC0) != 0x80) ++count;
    return count;
}
string column(const string& text, size_t width) {   // pad text to a fixed width (for tables)
    return visible(text) >= width ? text + " " : text + string(width - visible(text), ' ');
}
string stateColour(const string& state) {   // green = going well, yellow = waiting, magenta = paused, red = stopped
    return state == "Approved" || state == "Completed" ? "1;32" : state == "Accepted" || state == "Pending" ? "1;33"
         : state == "Suspended" ? "1;35" : "1;31";
}

// Approves every request whose time ran out and tells the user. Called before each screen is
// drawn, because the program waits for typing and has no other moment to look at the clock.
int announceAutoApprovals(WorkflowFacade& facade, ostream& out, bool colour) {
    int autoApproved = facade.processTimeouts();
    if (autoApproved > 0)
        out << "\n  " << paint(colour, "1;33", "*** " + to_string(autoApproved) +
                                                " request(s) auto-approved: nobody decided in time. ***") << "\n";
    return autoApproved;
}

// True when a line can be read without blocking. For the real keyboard this waits at most
// @p milliseconds, so the caller can look at the clock between waits. Any other stream (the
// unit tests type into a string) always has its input ready.
bool inputReady(istream& in, int milliseconds) {
#if defined(__unix__) || defined(__APPLE__)
    if (&in != &cin) return true;
    if (in.rdbuf()->in_avail() > 0) return true;   // text already read ahead (e.g. piped input)
    pollfd keyboard;
    keyboard.fd = STDIN_FILENO;
    keyboard.events = POLLIN;
    keyboard.revents = 0;
    int result = poll(&keyboard, 1, milliseconds);
    return result != 0 && !(result < 0 && errno == EINTR);   // also true at end of input, so getline sees it
#else
    (void)in; (void)milliseconds;
    return true;   // no polling on this platform: auto-approval then happens when the user types
#endif
}
}  // namespace

TerminalUI::TerminalUI(WorkflowFacade& facade, istream& in, ostream& out, bool colour)
    : facade(facade), in(in), out(out), colour(colour) {}

// ---------------------------------------------------------------- look and feel
string TerminalUI::paint(const string& code, const string& text) const { return ::paint(colour, code, text); }
void TerminalUI::heading(const string& title) {
    out << "\n" << paint("1;36", "── " + title + " " + repeat("─", WIDTH - 4 - (int)visible(title))) << "\n";
}
void TerminalUI::ok(const string& text) { out << "  " << paint("32", "✔ " + text) << "\n"; }
void TerminalUI::fail(const string& text) { out << "  " << paint("31", "✖ " + text) << "\n"; }
void TerminalUI::info(const string& text) { out << "  " << paint("36", "ℹ") << " " << text << "\n"; }

// ---------------------------------------------------------------- main menu
int TerminalUI::run() {
    out << "\n" << paint("36", "╭" + repeat("─", WIDTH - 2) + "╮") << "\n"
        << paint("36", "│") << paint("1", column("  TIVIDY - Workflow Management System", WIDTH - 2)) << paint("36", "│") << "\n"
        << paint("36", "│") << column("  Type your answer and press Enter. [brackets] = press Enter to accept.", WIDTH - 2) << paint("36", "│") << "\n"
        << paint("36", "╰" + repeat("─", WIDTH - 2) + "╯") << "\n";
    try {
        while (true) {
            // Requests nobody decided on in time may be approved automatically (lab tests).
            announceAutoApprovals(facade, out, colour);

            heading("MAIN MENU");
            out << "  " << paint("1;36", "1") << ". Start a new workflow        " << paint("2", "submit a lab test or loan request") << "\n"
                << "  " << paint("1;36", "2") << ". List all workflows          " << paint("2", "every request and its state") << "\n"
                << "  " << paint("1;36", "3") << ". Open a workflow             " << paint("2", "approve, deny, do tasks, report, undo") << "\n"
                << "  " << paint("1;36", "4") << ". Create a new workflow type  " << paint("2", "another industry, on the spot") << "\n"
                << "  " << paint("1;36", "5") << ". People and notifications    " << paint("2", "IDs, roles and their messages") << "\n"
                << "  " << paint("1;36", "6") << ". System audit log            " << paint("2", "every event in every workflow") << "\n"
                << "  " << paint("1;36", "0") << ". Exit\n";
            int choice = 0;
            try {
                choice = askNumber("Choose", "", true);
            } catch (const ScreenChanged&) {
                continue;   // something was auto-approved while waiting: draw the menu again
            }
            try {
                if (choice == 0) break;
                else if (choice == 1) startWorkflow();
                else if (choice == 2) listWorkflows();
                else if (choice == 3) openWorkflow(askNumber("Workflow number"));
                else if (choice == 4) createWorkflowType();
                else if (choice == 5) showPeople();
                else if (choice == 6) showAuditLog();
                else fail("Please choose a number from the menu.");
            } catch (const invalid_argument& error) {
                fail(string("Error: ") + error.what());   // e.g. a workflow number that does not exist
            }
        }
    } catch (const InputEnded&) {
    }
    out << "\n  " << paint("36", "Goodbye.") << "\n";
    return 0;
}

// ---------------------------------------------------------------- 1. start a workflow
void TerminalUI::startWorkflow(const string& presetType, int suggestedRequester) {
    heading("START A NEW WORKFLOW");
    string type = presetType;
    if (type.empty()) {
        vector<string> types = facade.listWorkflowTypes();
        out << "  Workflow types:\n";
        for (size_t i = 0; i < types.size(); ++i)
            out << "    " << paint("1;36", to_string(i + 1)) << ". " << column(types[i], 20)
                << paint("2", facade.getDefinition(types[i])->getDescription()) << "\n";
        int number = askNumber("Choose a type", "1");
        if (number < 1 || number > (int)types.size()) { fail("There is no such type."); return; }
        type = types[number - 1];
    }
    shared_ptr<const WorkflowDefinition> definition = facade.getDefinition(type);
    // Collect the answers. The Builder pattern turns them into a request.
    map<string, string> answers;
    if (suggestedRequester == 0) suggestedRequester = facade.listParticipants().front().getId();
    answers["requesterId"] = to_string(askPerson("Submitted by", suggestedRequester));
    answers["name"] = ask("Request title", "New " + type);
    for (size_t i = 0; i < definition->getRequestFields().size(); ++i) {
        string value = ask(definition->getRequestFields()[i].second);
        if (!value.empty()) answers[definition->getRequestFields()[i].first] = value;
    }
    bool urgent = askNumber("Priority: 1 = standard (14 days), 2 = urgent (2 days)", "1") == 2;

    // The same director builds a summary first, then the real request.
    RequestSummaryBuilder summary;
    WorkflowRequestDirector director(&summary);
    if (urgent) director.constructUrgent(answers); else director.constructStandard(answers);
    heading("SUMMARY");
    istringstream summaryLines(summary.getResult());
    for (string line; getline(summaryLines, line);) if (!trim(line).empty()) out << "    " << line << "\n";
    if (ask("Submit this request? (y/n)", "y") != "y") { info("Cancelled."); return; }
    WorkflowRequestBuilder builder;
    director.setBuilder(&builder);
    if (urgent) director.constructUrgent(answers); else director.constructStandard(answers);
    int id = facade.submitRequest(type, builder.getResult());
    string state = facade.getStatus(id);
    out << "  " << paint(state == "Rejected" ? "31" : "32", state == "Rejected" ? "✖" : "✔") << " Workflow #" << id
        << " created. State: " << paint(stateColour(state), state) << "\n";
    if (facade.getStatus(id) == "Rejected") showHistory(id, 1);   // shows why it was rejected
    if (ask("Open workflow #" + to_string(id) + " now? (y/n)", "y") == "y") openWorkflow(id);
}

// ---------------------------------------------------------------- 2. list workflows
void TerminalUI::listWorkflows() {
    announceAutoApprovals(facade, out, colour);
    vector<int> ids = facade.listWorkflows();
    heading("ALL WORKFLOWS");
    out << "  All workflows:\n";
    if (ids.empty()) { info("(none yet)"); return; }
    out << paint("2", "    " + column("#", 5) + column("Type", 20) + column("Title", 24) + column("Submitted by", 20) + "State") << "\n";
    for (size_t i = 0; i < ids.size(); ++i) {
        shared_ptr<const WorkflowInstance> wf = facade.getWorkflow(ids[i]);
        out << "    " << paint("1", column("#" + to_string(ids[i]), 5)) << column(wf->getDefinition()->getName(), 20)
            << column(wf->getRequest().getName(), 24) << column(facade.participantName(wf->getRequest().getRequesterId()), 20)
            << paint(stateColour(wf->getStateName()), wf->getStateName()) << "\n";
    }
}

// ---------------------------------------------------------------- 3. open a workflow
void TerminalUI::showWorkflow(int id) {
    shared_ptr<const WorkflowInstance> wf = facade.getWorkflow(id);
    const WorkflowRequest& request = wf->getRequest();
    heading("WORKFLOW #" + to_string(id) + " · " + wf->getDefinition()->getName());
    out << "  " << paint("2", "Request:   ") << paint("1", request.getName()) << " (deadline " << request.getDeadlineDays()
        << " days, priority " << request.getPriority() << ")\n"
        << "  " << paint("2", "Requester: ") << facade.participantName(request.getRequesterId()) << " (#" << request.getRequesterId() << ")\n"
        << "  " << paint("2", "State:     ") << paint(stateColour(wf->getStateName()), wf->getStateName()) << "\n"
        << "  " << paint("2", "Approval:  ") << wf->getDefinition()->getApproverRole() << " - " << wf->getApprovalPolicy()->describe() << "\n";
    // The work tree: stages (Composite) and their tasks.   ✓ done   ▶ available   · pending
    out << "\n  " << paint("1", "Tasks:") << "  " << paint("2", "(✓ done   ▶ available   · pending)") << "\n";
    const vector<shared_ptr<WorkItem> >& stages = wf->getRoot()->getChildren();
    for (size_t s = 0; s < stages.size(); ++s) {
        out << "    " << paint("1", stages[s]->getName()) << (stages[s]->isComplete() ? paint("32", " (done)") : "") << "\n";
        for (size_t t = 0; t < stages[s]->getChildren().size(); ++t) {
            shared_ptr<WorkItem> task = stages[s]->getChildren()[t];
            string status = task->getStatus() == "Approved" ? "Available" : task->getStatus();
            string who = task->getAssignee() >= 0 ? facade.participantName(task->getAssignee())
                                                  : "role " + task->getRequiredRole();
            string icon = status == "Completed" ? paint("32", "✓") : status == "Available" ? paint("33", "▶") : paint("2", "·");
            string shown = status == "Completed" ? paint("32", column(status, 10)) : status == "Available" ? paint("33", column(status, 10))
                                                                                                           : paint("2", column(status, 10));
            out << "      " << icon << " " << column("#" + to_string(task->getId()), 4) << column(task->getName(), 26) << shown << who << "\n";
        }
    }
    string report = facade.getReport(id);
    out << "\n  " << paint("1", "Report:") << "\n    " << (report.empty() ? paint("2", "(empty)") : report) << "\n";
    out << "\n  " << paint("1", "Recent events:") << "\n";
    showHistory(id, 5);
}

void TerminalUI::openWorkflow(int id) {
    facade.getWorkflow(id);   // stops here with an error if the number does not exist
    while (true) {
        announceAutoApprovals(facade, out, colour);
        showWorkflow(id);
        heading("ACTIONS");
        out << "  " << paint("1;36", "1") << ". Approve           " << paint("1;36", "2") << ". Deny                 " << paint("1;36", "3") << ". Suspend\n"
            << "  " << paint("1;36", "4") << ". Resume            " << paint("1;36", "5") << ". Complete a task      " << paint("1;36", "6") << ". Add report entry\n"
            << "  " << paint("1;36", "7") << ". Restore report revision                 " << paint("1;36", "8") << ". Undo last action\n"
            << "  " << paint("1;36", "9") << ". Full history                            " << paint("1;36", "0") << ". Back\n";
        int choice = 0;
        try {
            choice = askNumber("Choose", "", true);
        } catch (const ScreenChanged&) {
            continue;   // auto-approved while waiting: show the workflow again with its new state
        }
        if (choice == 0) return;
        announceAutoApprovals(facade, out, colour);   // time may have run out while the user was deciding
        shared_ptr<const WorkflowInstance> wf = facade.getWorkflow(id);
        string approverRole = wf->getDefinition()->getApproverRole();
        int requester = wf->getRequest().getRequesterId();
        bool done = false;   string refusal;

        if (choice == 1 || choice == 2) {
            int actor = askPerson(choice == 1 ? "Approve as" : "Deny as", firstWithRole(approverRole));
            if (choice == 1) done = facade.approveRequest(id, actor);
            else done = facade.rejectRequest(id, actor, ask("Reason", "Does not meet the requirements"));
            refusal = "only a " + approverRole + " can decide, only while the request is Accepted, "
                      "and never on their own request";
        } else if (choice == 3 || choice == 4) {
            int actor = askPerson(choice == 3 ? "Suspend as" : "Resume as", firstWithRole(approverRole));
            done = choice == 3 ? facade.suspendRequest(id, actor) : facade.resumeRequest(id, actor);
            refusal = "only a " + approverRole + " can do this, and only Approved work can be suspended "
                      "or Suspended work resumed";
        } else if (choice == 5) {
            vector<shared_ptr<WorkItem> > available = facade.getAvailableWork(id);
            if (available.empty()) { fail("No tasks can be done now (work happens only while Approved)."); continue; }
            out << "\n  Tasks you can do now:\n";
            for (size_t i = 0; i < available.size(); ++i)
                out << "    " << paint("33", "▶") << " " << column("#" + to_string(available[i]->getId()), 4) << column(available[i]->getName(), 26)
                    << "assigned to " << facade.participantName(available[i]->getAssignee()) << " (#" << available[i]->getAssignee() << ")\n";
            int taskId = askNumber("Task number", to_string(available[0]->getId()));
            int suggested = available[0]->getAssignee();
            for (size_t i = 0; i < available.size(); ++i)
                if (available[i]->getId() == taskId) suggested = available[i]->getAssignee();
            done = facade.completeWorkItem(id, taskId, askPerson("Completed by", suggested));
            refusal = "the task must be in the list above, and only the assigned person can complete it";
        } else if (choice == 6) {
            int author = askPerson("Written by", requester);
            string entry = ask("Entry");
            string current = facade.getReport(id);
            done = !entry.empty() &&
                   facade.updateReport(id, author, current + (current.empty() ? "" : "\n") +
                                                       facade.participantName(author) + ": " + entry);
            refusal = "the entry was empty, or the report is locked (suspended or finished workflow)";
        } else if (choice == 7) {
            vector<string> revisions = facade.getReportRevisions(id);
            if (revisions.empty()) { fail("No revisions saved yet."); continue; }
            for (size_t i = 0; i < revisions.size(); ++i) out << "    " << revisions[i] << "\n";
            done = facade.restoreReport(id, askNumber("Revision index to restore"), requester);
            refusal = "no such revision, or the report is locked";
        } else if (choice == 8) {
            done = facade.undoLastAction(id);
            refusal = "approvals and denials are final, and finished workflows cannot change";
        } else if (choice == 9) {
            heading("FULL HISTORY OF WORKFLOW #" + to_string(id));
            showHistory(id, 0);
            ask("Press Enter to continue");
            continue;
        } else {
            fail("Please choose a number from the list.");
            continue;
        }
        if (done) ok("Done.");
        else fail("Not allowed: " + refusal + ".");
    }
}

void TerminalUI::showHistory(int id, size_t last) {
    vector<WorkflowEvent> history = facade.getHistory(id);
    size_t from = (last > 0 && history.size() > last) ? history.size() - last : 0;
    for (size_t i = from; i < history.size(); ++i)
        out << "    " << paint("2", history[i].getTimestamp()) << "  " << paint("1", history[i].getType() + ":") << " "
            << history[i].getMessage() << "\n";
}

// ---------------------------------------------------------------- 4. create a workflow type on the spot
void TerminalUI::createWorkflowType() {
    heading("CREATE A NEW WORKFLOW TYPE");
    info("Create a new workflow type. Press Enter to accept a suggestion in [brackets].");
    string name = ask("Name", "Equipment Purchase");
    string description = ask("Description", "Request, approve, buy and receive equipment");
    string approverRole = ask("Role that approves requests", "Manager");
    int seconds = askNumber("Auto-approve after how many seconds? (-1 = a person must always decide)", "-1");
    vector<string> fields = splitByComma(ask("Request fields, separated by commas", "item, quantity, cost"));
    string limitField;   double limitMax = 0;
    istringstream limit(ask("Numeric limit as 'field maximum' (type none for no limit)", "cost 50000"));
    if (!(limit >> limitField >> limitMax)) limitField = "";
    // Stages, each with tasks written as "Task name (Role)". The role Requester means whoever submitted.
    const char* suggestedStages[2][2] = {{"Procurement", "Get quotes (Buyer), Place order (Buyer)"},
                                         {"Delivery", "Receive goods (Requester)"}};
    vector<pair<string, vector<string> > > stages;   vector<bool> inOrder;
    for (int s = 0;; ++s) {
        string stage = ask("Stage " + to_string(s + 1) + " name (type done to finish)", s < 2 ? suggestedStages[s][0] : "done");
        if (stage == "done") break;
        vector<string> tasks = splitByComma(ask("Its tasks as 'Task (Role)', separated by commas",
                                                s < 2 ? suggestedStages[s][1] : ""));
        inOrder.push_back(ask("Must these tasks be done in order? (y/n)", "y") == "y");
        stages.push_back(make_pair(stage, tasks));
    }
    // Make sure somebody holds every role the workflow needs.
    vector<string> roles(1, approverRole);
    for (size_t s = 0; s < stages.size(); ++s)
        for (size_t t = 0; t < stages[s].second.size(); ++t) {
            const string& task = stages[s].second[t];
            size_t open = task.rfind('('), close = task.rfind(')');
            if (open != string::npos && close != string::npos && close > open)
                roles.push_back(task.substr(open + 1, close - open - 1));
        }
    for (size_t r = 0; r < roles.size(); ++r)
        if (roles[r] != "Requester" && firstWithRole(roles[r]) == -1) addPerson(roles[r], "New " + roles[r]);
    int submitter = addPerson("Staff", "Lindiwe Zulu");
    configureCustomWorkflow(facade, name, description, approverRole, seconds, fields, limitField, limitMax,
                            stages, inOrder);
    ok("Workflow type '" + name + "' created. It runs on the same engine as the other workflows.");
    if (ask("Start a '" + name + "' workflow now? (y/n)", "y") == "y") startWorkflow(name, submitter);
}

int TerminalUI::addPerson(const string& role, const string& suggestedName) {
    vector<Participant> everyone = facade.listParticipants();
    int id = 300;
    for (size_t i = 0; i < everyone.size(); ++i)
        if (everyone[i].getId() >= id) id = everyone[i].getId() + 1;
    string name = ask("Name of a person with role '" + role + "'", suggestedName);
    facade.registerParticipant(Participant(id, name, vector<string>(1, role)));
    ok("Added " + name + " as person #" + to_string(id) + ".");
    return id;
}

// ---------------------------------------------------------------- 5 and 6. people, audit log
void TerminalUI::showPeople() {
    announceAutoApprovals(facade, out, colour);
    vector<Participant> everyone = facade.listParticipants();
    heading("PEOPLE AND NOTIFICATIONS");
    out << "  People:\n";
    out << paint("2", "    " + column("ID", 7) + column("Name", 20) + column("Roles", 28) + column("Open tasks", 12) + "Messages") << "\n";
    for (size_t i = 0; i < everyone.size(); ++i)
        out << "    " << paint("1", column("#" + to_string(everyone[i].getId()), 7)) << column(everyone[i].getName(), 20)
            << column(everyone[i].rolesText(), 28) << column(to_string(everyone[i].getWorkload()), 12)
            << facade.getNotifications(everyone[i].getId()).size() << "\n";
    string answer = ask("Show messages for which person id? (-1 = approvers, Enter = back)");
    if (answer.empty()) return;
    vector<string> messages = facade.getNotifications(atoi(answer.c_str()));
    if (messages.empty()) info("(no messages)");
    for (size_t i = 0; i < messages.size(); ++i) out << "    " << paint("34", "✉") << " " << messages[i] << "\n";
}

void TerminalUI::showAuditLog() {
    announceAutoApprovals(facade, out, colour);
    vector<string> log = facade.getAuditLog();
    heading("AUDIT LOG");
    out << "  System audit log (" << log.size() << " events):\n";
    for (size_t i = 0; i < log.size(); ++i) out << "    " << log[i] << "\n";
}

// ---------------------------------------------------------------- input helpers
string TerminalUI::ask(const string& question, const string& suggestion, bool refreshOnChange) {
    string prompt = "  " + paint("1;36", "?") + " " + paint("1", question);
    if (!suggestion.empty()) prompt += " " + paint("32", "[" + suggestion + "]");
    prompt += ": ";
    out << prompt;
    // While nobody types, keep checking whether a request has run out of time.
    out.flush();
    while (!inputReady(in, 200)) {
        if (announceAutoApprovals(facade, out, colour) == 0) continue;
        if (refreshOnChange) throw ScreenChanged();   // a menu: draw the screen again
        out << prompt;                                // a question in the middle of a task: ask it again
        out.flush();
    }
    string answer;
    if (!getline(in, answer)) throw InputEnded();
    answer = trim(answer);
    return answer.empty() ? suggestion : answer;
}

int TerminalUI::askNumber(const string& question, const string& suggestion, bool refreshOnChange) {
    while (true) {
        istringstream answer(ask(question, suggestion, refreshOnChange));
        int number;   string extra;
        if (answer >> number && !(answer >> extra)) return number;
        fail("Please type a number.");
    }
}

int TerminalUI::askPerson(const string& question, int suggestedId) {
    while (true) {
        int id = askNumber(question + " (person id)", suggestedId > 0 ? to_string(suggestedId) : "");
        if (facade.participantName(id) != "#" + to_string(id)) return id;
        fail("Nobody has id " + to_string(id) + ". Menu option 5 lists everyone.");
    }
}

int TerminalUI::firstWithRole(const string& role) {
    vector<Participant> everyone = facade.listParticipants();
    for (size_t i = 0; i < everyone.size(); ++i)
        if (everyone[i].hasRole(role)) return everyone[i].getId();
    return -1;
}