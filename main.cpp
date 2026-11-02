/**
 * @file main.cpp
 * @brief Starts TIVIDY: creates the engine and its helpers, sets up the industries, shows the menu.
 *
 * ./main      lab requests are auto-approved after 30 seconds without a decision
 * ./main 5    the same after 5 seconds (handy during a demo)
 */
#include <cstdlib>
#include <iostream>
#include <memory>
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#define ON_A_REAL_TERMINAL isatty(STDOUT_FILENO)
#else
#define ON_A_REAL_TERMINAL false
#endif

#include "CommandInvoker.h"
#include "DefaultWorkflowMediator.h"
#include "IndustrySetup.h"
#include "TerminalUI.h"
#include "WorkflowEngine.h"
#include "WorkflowFacade.h"

int main(int argc, char* argv[]) {
    // Lets the menu check for timed-out requests while it waits for typing (see TerminalUI::ask).
    std::ios::sync_with_stdio(false);
    int labTimeoutSeconds = argc > 1 ? std::atoi(argv[1]) : 30;

    // The generic engine and the objects around it. Nothing here belongs to one industry.
    std::shared_ptr<WorkflowEngine> engine = std::make_shared<WorkflowEngine>();
    std::shared_ptr<CommandInvoker> invoker = std::make_shared<CommandInvoker>();
    std::shared_ptr<WorkflowMediator> mediator = std::make_shared<DefaultWorkflowMediator>(*engine);
    WorkflowFacade facade(engine, invoker, mediator);

    // The industries. Everything industry-specific is in IndustrySetup.cpp.
    configureAllIndustries(facade, labTimeoutSeconds);

    // Colour only on a real terminal (never in files, pipes or tests). NO_COLOR=1 switches it off.
    bool colour = ON_A_REAL_TERMINAL && std::getenv("NO_COLOR") == 0;
    TerminalUI menu(facade, std::cin, std::cout, colour);
    return menu.run();
}