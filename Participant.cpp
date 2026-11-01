/**
 * @file Participant.cpp
 * @brief Implementation of Participant.
 */
#include "Participant.h"

Participant::Participant() : id_(0), currentWorkload_(0) {}

Participant::Participant(int id, const std::string& name, const std::vector<std::string>& roles)
    : id_(id), name_(name), roles_(roles), currentWorkload_(0) {}

int Participant::getId() const { return id_; }

std::string Participant::getName() const { return name_; }

const std::vector<std::string>& Participant::getRoles() const { return roles_; }

std::string Participant::rolesText() const {
    std::string out;
    for (std::size_t i = 0; i < roles_.size(); ++i) {
        if (i) out += ", ";
        out += roles_[i];
    }
    return out;
}

bool Participant::hasRole(const std::string& role) const {
    for (std::size_t i = 0; i < roles_.size(); ++i)
        if (roles_[i] == role) return true;
    return false;
}

int Participant::getWorkload() const { return currentWorkload_; }

void Participant::addWork() { ++currentWorkload_; }

void Participant::finishWork() {
    if (currentWorkload_ > 0) --currentWorkload_;
}
