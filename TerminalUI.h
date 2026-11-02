/**
 * @file TerminalUI.h
 * @brief The text menu the user works with. It only talks to the WorkflowFacade.
 */
#ifndef TERMINALUI_H
#define TERMINALUI_H

#include <iostream>
#include <string>

class WorkflowFacade;

/**
 * @brief Text menu (TUI) for TIVIDY.
 *
 * Every menu option calls one WorkflowFacade operation. The input and output
 * streams are parameters so the unit tests can type into the menu. Colour is
 * optional and only changes how things look.
 */
class TerminalUI {
public:
    /** @param colour Use colours (main() turns them on only for a real terminal; tests leave them off). */
    TerminalUI(WorkflowFacade& facade, std::istream& in, std::ostream& out, bool colour = false);
    /** @brief Show the main menu until the user chooses 0 or the input ends. @return 0. */
    int run();

private:
    void startWorkflow(const std::string& presetType = "", int suggestedRequester = 0);
    void listWorkflows();
    void openWorkflow(int id);
    void showWorkflow(int id);
    void createWorkflowType();
    void showPeople();
    void showAuditLog();
    void showHistory(int id, std::size_t last);

    // refreshOnChange: used by the menus. While waiting for typing, keep checking for timed-out requests.
    std::string ask(const std::string& question, const std::string& suggestion = "", bool refreshOnChange = false);
    int askNumber(const std::string& question, const std::string& suggestion = "", bool refreshOnChange = false);
    int askPerson(const std::string& question, int suggestedId);
    int firstWithRole(const std::string& role);
    int addPerson(const std::string& role, const std::string& suggestedName);

    // Look and feel only: colour, headings and the ✔ / ✖ / ℹ message lines.
    std::string paint(const std::string& code, const std::string& text) const;
    void heading(const std::string& title);
    void ok(const std::string& text);
    void fail(const std::string& text);
    void info(const std::string& text);

    WorkflowFacade& facade;
    std::istream& in;
    std::ostream& out;
    bool colour;
};

#endif