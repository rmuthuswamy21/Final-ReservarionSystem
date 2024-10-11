#include "User.h"
#include "Reservation.h"
#include "Ticket.h"
#include "Event.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

User::User(const std::string& username, const std::string& password, UserType type)
    : username(username), password(password), type(type), budget(1000.0) {} // Start with $1000 budget

bool User::login(const std::string& inputUsername, const std::string& inputPassword) const {
    return (inputUsername == username && inputPassword == password);
}

bool User::makeReservation(std::unique_ptr<Reservation> reservation) {
    double cost = reservation->getTotalCost();
    if (budget >= cost) {
        reservations.push_back(std::move(reservation));
        budget -= cost;
        return true;
    }
    return false;
}

bool User::cancelReservation(const std::string& reservationId) {
    auto it = std::find_if(reservations.begin(), reservations.end(),
        [&reservationId](const std::unique_ptr<Reservation>& res) { return res->getId() == reservationId; });
    if (it != reservations.end()) {
        double refundAmount = (*it)->calculateRefund();
        addToBudget(refundAmount);
        reservations.erase(it);
        return true;
    }
    return false;
}

bool User::purchaseTicket(std::unique_ptr<Ticket> ticket) {
    if (deductFromBudget(ticket->getPrice())) {
        tickets.push_back(std::move(ticket));
        return true;
    }
    return false;
}

void User::viewSchedule() const {
    std::cout << "Schedule for " << username << ":" << std::endl;
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

void User::addToBudget(double amount) {
    budget += amount;
}

bool User::deductFromBudget(double amount) {
    if (budget >= amount) {
        budget -= amount;
        return true;
    }
    return false;
}

void User::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << username << std::endl;
        file << password << std::endl;
        file << static_cast<int>(type) << std::endl;
        file << budget << std::endl;

        // Save reservations
        file << reservations.size() << std::endl;
        for (const auto& reservation : reservations) {
            file << reservation->getId() << std::endl;
            file << reservation->getStartTime().time_since_epoch().count() << std::endl;
            file << reservation->getEndTime().time_since_epoch().count() << std::endl;
            file << static_cast<int>(reservation->getLayoutStyle()) << std::endl;
            file << reservation->getIsPublic() << std::endl;
        }

        // Save tickets
        file << tickets.size() << std::endl;
        for (const auto& ticket : tickets) {
            file << ticket->getEvent()->getName() << std::endl;
            file << ticket->getPrice() << std::endl;
        }

        file.close();
        std::cout << "User data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

void User::loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<Event>>& events) {
    std::ifstream file(filename);
    if (file.is_open()) {
        file >> username;
        file >> password;
        int typeInt;
        file >> typeInt;
        type = static_cast<UserType>(typeInt);
        file >> budget;

        // Load reservations
        size_t reservationCount;
        file >> reservationCount;
        reservations.clear();
        for (size_t i = 0; i < reservationCount; ++i) {
            std::string id;
            long long startTime, endTime;
            int layoutStyleInt;
            bool isPublic;
            file >> id >> startTime >> endTime >> layoutStyleInt >> isPublic;
            auto start = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(startTime));
            auto end = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(endTime));
            reservations.push_back(std::unique_ptr<Reservation>(new Reservation(this, start, end, static_cast<LayoutStyle>(layoutStyleInt), isPublic)));
            reservations.back()->setId(id);
        }

        // Load tickets
        size_t ticketCount;
        file >> ticketCount;
        tickets.clear();
        for (size_t i = 0; i < ticketCount; ++i) {
            std::string eventName;
            double price;
            file >> eventName >> price;
            auto it = std::find_if(events.begin(), events.end(),
                [&eventName](const std::unique_ptr<Event>& e) { return e->getName() == eventName; });
            if (it != events.end()) {
                tickets.push_back(std::unique_ptr<Ticket>(new Ticket(it->get(), this, price)));
            }
        }

        file.close();
        std::cout << "User data loaded from file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

void User::displayUserInfo() const {
    std::cout << "User Information:" << std::endl;
    std::cout << "Username: " << username << std::endl;
    std::cout << "User Type: ";
    switch (type) {
        case UserType::Resident:
            std::cout << "Resident";
            break;
        case UserType::NonResident:
            std::cout << "Non-Resident";
            break;
        case UserType::City:
            std::cout << "City User";
            break;
        case UserType::Organization:
            std::cout << "Organization";
            break;
    }
    std::cout << std::endl;
    std::cout << "Budget: $" << std::fixed << std::setprecision(2) << budget << std::endl;
    viewSchedule();
}