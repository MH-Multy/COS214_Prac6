/**
 * @file WorkflowRequestDirector.h
 * @brief Builder Director: the fixed sequences for building standard and urgent requests.
 */
#ifndef WORKFLOWREQUESTDIRECTOR_H
#define WORKFLOWREQUESTDIRECTOR_H

#include "WorkflowBuilder.h"
#include <map>
#include <string>

using namespace std;

/**
 * @brief Builder Director: knows the order of construction steps, not what they produce.
 * @ingroup builder
 *
 * Works only through the abstract WorkflowBuilder, so any concrete builder can
 * be plugged in (or swapped with setBuilder()) without changing the director.
 *
 * Input is a generic map from the user interface. The keys "name" and
 * "requesterId" fill the header; every other key becomes an attribute.
 */
class WorkflowRequestDirector{
    private:
        WorkflowBuilder* builder;  ///< Not owned; must outlive the director's use of it.
    public:
        /** @param b The builder to drive (not owned). */
        WorkflowRequestDirector(WorkflowBuilder* b);

        /** @brief Switch to a different builder. @param b The builder to drive (not owned). */
        void setBuilder(WorkflowBuilder* b);

        /**
         * @brief Build a standard request: 14-day deadline, priority 1.
         * @param input Field values from the user interface.
         * @throws std::invalid_argument if there is no builder, or "requesterId" is not a number.
         */
        void constructStandard(const map<string, string>& input);

        /**
         * @brief Build an urgent request: 2-day deadline, priority 5.
         * @param input Field values from the user interface.
         * @throws std::invalid_argument if there is no builder, or "requesterId" is not a number.
         */
        void constructUrgent(const map<string, string>& input);
};

#endif //WORKFLOWREQUESTDIRECTOR_H
