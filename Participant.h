/**
 * @file Participant.h
 * @brief A person who submits, approves or carries out workflow work.
 */
#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string>
#include <vector>

/**
 * @brief A person who takes part in workflows: requester, approver or worker.
 * @ingroup core
 *
 * Roles decide what a participant may do (e.g. "HeadResearcher" may approve
 * lab tests). The workload counts tasks currently assigned, and is used to
 * route new work to the least busy participant with the right role.
 */
class Participant {
public:
    /** @brief An empty participant (id 0); needed for use in standard containers. */
    Participant();

    /**
     * @param id    Unique participant id.
     * @param name  Display name.
     * @param roles Roles held, e.g. {"Researcher"}.
     */
    Participant(int id, const std::string& name, const std::vector<std::string>& roles);

    /** @brief Participant id. @return The id. */
    int getId() const;
    /** @brief Display name. @return The name. */
    std::string getName() const;
    /** @brief All roles held. @return The roles. */
    const std::vector<std::string>& getRoles() const;
    /** @brief Roles as one comma-separated line. @return e.g. "Researcher, LabTechnician". */
    std::string rolesText() const;

    /** @brief Whether the participant holds a role. @param role Role name. @return true if held. */
    bool hasRole(const std::string& role) const;

    /** @brief Tasks currently assigned and not finished. @return The workload. */
    int getWorkload() const;
    /** @brief Record that a task was assigned. */
    void addWork();
    /** @brief Record that an assigned task was finished (never goes below zero). */
    void finishWork();

private:
    int id_;
    std::string name_;
    std::vector<std::string> roles_;
    int currentWorkload_;
};

#endif
