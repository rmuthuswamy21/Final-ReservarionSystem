#include "Resident.h"
#include "Event.h"
#include "Reservation.h"
#include "Ticket.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

Resident::Resident(const std::string& username, const std::string& password)
    : User(username, password, UserType::Resident) {}

bool Resident::makeReservation(std::unique_ptr<Reservation> reservation) {
    // Check if the resident has reached the weekly limit (24 hours)
    auto now = std::chrono::system_clock::now();
    auto oneWeekAgo = now - std::chrono::hours(24 * 7);
    
    std::chrono::duration<double, std::ratio<3600>> totalHours(0);
    for (const auto& res : reservations) {
        if (res->getStartTime() >= oneWeekAgo) {
            totalHours += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(
                res->getEndTime() - res->getStartTime());
        }
    }
    
    totalHours += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(
        reservation->getEndTime() - reservation->getStartTime());
    
    if (totalHours.count() > 24) {
        std::cout << "Reservation exceeds weekly limit of 24 hours for residents." << std::endl;
        return false;
    }
    
    double cost = reservation->getTotalCost();
    if (deductFromBudget(cost)) {
        std::cout << "Making reservation for Resident: " << getUsername() << std::endl;
        reservations.push_back(std::move(reservation));
        return true;
    } else {
        std::cout << "Insufficient funds to make the reservation." << std::endl;
        return false;
    }
}

bool Resident::cancelReservation(const std::string& reservationId) {
    auto it = std::find_if(reservations.begin(), reservations.end(),
        [&reservationId](const std::unique_ptr<Reservation>& r) { return r->getId() == reservationId; });

    if (it != reservations.end()) {
        double refundAmount = (*it)->calculateRefund();
        addToBudget(refundAmount);
        std::cout << "Canceling reservation for Resident: " << getUsername() << std::endl;
        std::cout << "Refund amount: $" << refundAmount << std::endl;
        reservations.erase(it);
        return true;
    }
    std::cout << "Reservation not found." << std::endl;
    return false;
}

bool Resident::purchaseTicket(std::unique_ptr<Ticket> ticket) {
    if (!ticket->getEvent()->getAllowsResidents()) {
        std::cout << "This event is not open to residents." << std::endl;
        return false;
    }
    
    if (deductFromBudget(ticket->getPrice())) {
        std::cout << "Purchasing ticket for Resident: " << getUsername() << std::endl;
        tickets.push_back(std::move(ticket));
        return true;
    } else {
        std::cout << "Insufficient funds to purchase the ticket." << std::endl;
        return false;
    }
}

void Resident::viewSchedule() const {
    std::cout << "Schedule for Resident: " << getUsername() << std::endl;
    if (reservations.empty()) {
        std::cout << "No reservations scheduled." << std::endl;
    } else {
        for (const auto& reservation : reservations) {
            std::cout << reservation->toString() << std::endl;
        }
    }
    
    if (tickets.empty()) {
        std::cout << "No tickets purchased." << std::endl;
    } else {
        std::cout << "Purchased Tickets:" << std::endl;
        for (const auto& ticket : tickets) {
            std::cout << ticket->toString() << std::endl;
        }
    }
}

void Resident::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << getUsername() << std::endl;
        file << "password" << std::endl; // We don't store actual passwords
        file << static_cast<int>(getType()) << std::endl;
        file << getBudget() << std::endl;

        // Save reservations
        file << reservations.size() << std::endl;
        for (const auto& reservation : reservations) {
            file << reservation->getId() << std::endl;
            reservation->saveToFile(filename + "_" + reservation->getId());
        }

        // Save tickets
        file << tickets.size() << std::endl;
        for (const auto& ticket : tickets) {
            file << ticket->getId() << std::endl;
            ticket->saveToFile(filename + "_" + ticket->getId());
        }

        file.close();
        std::cout << "Resident data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

void Resident::loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<Event>>& events) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string username, password;
        int typeInt;
        double budget;
        
        std::getline(file, username);
        std::getline(file, password);
        file >> typeInt >> budget;

        // We don't update the username, password, or type as they're set in the constructor
        addToBudget(budget);

        // Load reservations
        int numReservations;
        file >> numReservations;
        for (int i = 0; i < numReservations; ++i) {
            std::string reservationId;
            file >> reservationId;
            auto reservation = Reservation::loadFromFile(filename + "_" + reservationId, this);
            if (reservation) {
                reservations.push_back(std::move(reservation));
            }
        }

        // Load tickets
        int numTickets;
        file >> numTickets;
        for (int i = 0; i < numTickets; ++i) {
            std::string ticketId;
            file >> ticketId;
            std::vector<std::unique_ptr<User>> users;
            users.push_back(std::unique_ptr<User>(this));
            auto ticket = Ticket::loadFromFile(filename + "_" + ticketId, events, users);
            if (ticket) {
                tickets.push_back(std::move(ticket));
            }
        }

        file.close();
        std::cout << "Resident data loaded from file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}