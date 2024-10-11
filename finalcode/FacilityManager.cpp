#include "FacilityManager.h"
#include "Facility.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>

FacilityManager::FacilityManager(Facility& facility) : facility(facility), budget(0.0) {}

bool FacilityManager::approveReservation(const std::string& reservationId) {
    auto it = std::find_if(pendingReservations.begin(), pendingReservations.end(),
        [&reservationId](const std::unique_ptr<Reservation>& r) { return r->getId() == reservationId; });
    
    if (it != pendingReservations.end()) {
        if ((*it)->getIsPaid()) {
            if (facility.isAvailable((*it)->getStartTime(), (*it)->getEndTime())) {
                if (checkReservationLimits((*it)->getUser(), it->get())) {
                    facility.addReservation(std::move(*it));
                    updateBudget((*it)->getTotalCost());
                    std::cout << "Reservation " << reservationId << " approved and added to the facility schedule." << std::endl;
                    pendingReservations.erase(it);
                    return true;
                } else {
                    std::cout << "Reservation exceeds user's weekly limit." << std::endl;
                }
            } else {
                std::cout << "The facility is not available for the requested time slot." << std::endl;
            }
        } else {
            std::cout << "Cannot approve unpaid reservation." << std::endl;
        }
    } else {
        std::cout << "Reservation not found." << std::endl;
    }
    return false;
}

bool FacilityManager::rejectReservation(const std::string& reservationId) {
    auto it = std::find_if(pendingReservations.begin(), pendingReservations.end(),
        [&reservationId](const std::unique_ptr<Reservation>& r) { return r->getId() == reservationId; });
    
    if (it != pendingReservations.end()) {
        if ((*it)->getIsPaid()) {
            processRefund((*it)->getTotalCost());
        }
        std::cout << "Reservation " << reservationId << " rejected." << std::endl;
        pendingReservations.erase(it);
        return true;
    }
    std::cout << "Reservation not found." << std::endl;
    return false;
}

void FacilityManager::processRefund(double amount) {
    budget -= amount;
    std::cout << "Refund processed: $" << amount << std::endl;
}

void FacilityManager::updateBudget(double amount) {
    budget += amount;
    std::cout << "Budget updated. New balance: $" << budget << std::endl;
}

bool FacilityManager::checkReservationLimits(const User* user, const Reservation* newReservation) const {
    auto now = std::chrono::system_clock::now();
    auto oneWeekAgo = now - std::chrono::hours(24 * 7);
    
    std::chrono::duration<double, std::ratio<3600>> totalHours(0);

    for (const auto& reservation : facility.getReservations()) {
        if (reservation->getUser() == user && reservation->getStartTime() >= oneWeekAgo) {
            totalHours += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(
                reservation->getEndTime() - reservation->getStartTime());
        }
    }

    totalHours += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(
        newReservation->getEndTime() - newReservation->getStartTime());

    switch (user->getType()) {
        case UserType::City:
            return totalHours.count() <= 48;
        case UserType::Organization:
            return totalHours.count() <= 36;
        default:
            return totalHours.count() <= 24;
    }
}

void FacilityManager::displayPendingReservations() const {
    std::cout << "Pending Reservations:" << std::endl;
    if (pendingReservations.empty()) {
        std::cout << "No pending reservations." << std::endl;
    } else {
        for (const auto& reservation : pendingReservations) {
            std::cout << reservation->toString() << std::endl;
        }
    }
}

void FacilityManager::displayBudget() const {
    std::cout << "Current Budget: $" << budget << std::endl;
}

void FacilityManager::addPendingReservation(std::unique_ptr<Reservation> reservation) {
    pendingReservations.push_back(std::move(reservation));
    std::cout << "Reservation added to pending list. Waiting for approval." << std::endl;
}

bool FacilityManager::isAvailable(const std::chrono::system_clock::time_point& startTime, 
                                  const std::chrono::system_clock::time_point& endTime) const {
    return facility.isAvailable(startTime, endTime);
}

void FacilityManager::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << budget << std::endl;
        file << pendingReservations.size() << std::endl;
        for (const auto& reservation : pendingReservations) {
            file << reservation->getId() << std::endl;
            file << reservation->getUser()->getUsername() << std::endl;  // Save the username
            reservation->saveToFile(filename + "_" + reservation->getId());
        }
        file.close();
        std::cout << "FacilityManager data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

void FacilityManager::loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<User>>& users) {
    std::ifstream file(filename);
    if (file.is_open()) {
        file >> budget;
        size_t pendingCount;
        file >> pendingCount;
        for (size_t i = 0; i < pendingCount; ++i) {
            std::string reservationId;
            file >> reservationId;
            
            // Read the username associated with this reservation
            std::string username;
            file >> username;
            
            // Find the user
            auto userIt = std::find_if(users.begin(), users.end(),
                [&username](const std::unique_ptr<User>& u) { return u->getUsername() == username; });
            
            if (userIt != users.end()) {
                // Load the reservation with the correct user
                auto reservation = Reservation::loadFromFile(filename + "_" + reservationId, userIt->get());
                if (reservation) {
                    pendingReservations.push_back(std::move(reservation));
                }
            } else {
                std::cout << "User not found for reservation: " << reservationId << std::endl;
            }
        }
        file.close();
        std::cout << "FacilityManager data loaded from file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}