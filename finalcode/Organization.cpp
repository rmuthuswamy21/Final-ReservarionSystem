#include "Organization.h"
#include "Reservation.h"
#include "Ticket.h"
#include "Event.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

Organization::Organization(const std::string& username, const std::string& password)
    : User(username, password, UserType::Organization) {}

bool Organization::makeReservation(std::unique_ptr<Reservation> reservation) {
    // Check if the organization has reached the weekly limit (36 hours)
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
    
    if (totalHours.count() > 36) {
        std::cout << "Reservation exceeds weekly limit of 36 hours for organizations." << std::endl;
        return false;
    }
    
    // Check if the layout style is valid for organizations
    if (!isValidLayoutStyle(reservation->getLayoutStyle())) {
        std::cout << "Invalid layout style for organization. Only Meeting, Lecture, and Dance styles are allowed." << std::endl;
        return false;
    }
    
    double cost = reservation->getTotalCost();
    if (deductFromBudget(cost)) {
        std::cout << "Making reservation for Organization: " << getUsername() << std::endl;
        reservations.push_back(std::move(reservation));
        return true;
    } else {
        std::cout << "Insufficient funds to make the reservation." << std::endl;
        return false;
    }
}

bool Organization::cancelReservation(const std::string& reservationId) {
    auto it = std::find_if(reservations.begin(), reservations.end(),
        [&reservationId](const std::unique_ptr<Reservation>& r) { return r->getId() == reservationId; });

    if (it != reservations.end()) {
        double refundAmount = (*it)->calculateRefund();
        addToBudget(refundAmount);
        std::cout << "Canceling reservation for Organization: " << getUsername() << std::endl;
        std::cout << "Refund amount: $" << refundAmount << std::endl;
        reservations.erase(it);
        return true;
    }
    std::cout << "Reservation not found." << std::endl;
    return false;
}

bool Organization::purchaseTicket(std::unique_ptr<Ticket> ticket) {
    if (deductFromBudget(ticket->getPrice())) {
        std::cout << "Purchasing ticket for Organization: " << getUsername() << std::endl;
        tickets.push_back(std::move(ticket));
        return true;
    } else {
        std::cout << "Insufficient funds to purchase the ticket." << std::endl;
        return false;
    }
}

void Organization::viewSchedule() const {
    std::cout << "Schedule for Organization: " << getUsername() << std::endl;
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

void Organization::saveToFile(const std::string& filename) const {
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
        std::cout << "Organization data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

void Organization::loadFromFile(const std::string& filename, 
                                const std::vector<std::unique_ptr<Event>>& events) {
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
        std::cout << "Organization data loaded from file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

bool Organization::isValidLayoutStyle(LayoutStyle style) const {
    return style == LayoutStyle::Meeting || style == LayoutStyle::Lecture || style == LayoutStyle::Dance;
}