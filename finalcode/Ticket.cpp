#include "Ticket.h"
#include "Event.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

int Ticket::nextId = 1;

Ticket::Ticket(Event* event, User* user, double price)
    : event(event), user(user), price(price), isValid(true) {
    std::stringstream ss;
    ss << "TKT" << std::setw(4) << std::setfill('0') << nextId++;
    id = ss.str();
}

bool Ticket::cancel() {
    if (isValid) {
        isValid = false;
        user->addToBudget(price); // Refund the ticket price to the user's budget
        std::cout << "Ticket " << id << " has been cancelled and refunded." << std::endl;
        return true;
    }
    std::cout << "Ticket " << id << " is already cancelled or invalid." << std::endl;
    return false;
}

std::string Ticket::toString() const {
    std::stringstream ss;
    ss << "Ticket ID: " << id << std::endl
       << "Event: " << event->getName() << std::endl
       << "User: " << user->getUsername() << std::endl
       << "Price: $" << std::fixed << std::setprecision(2) << price << std::endl
       << "Status: " << (isValid ? "Valid" : "Cancelled");
    return ss.str();
}

void Ticket::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << id << std::endl;
        file << event->getName() << std::endl;
        file << user->getUsername() << std::endl;
        file << price << std::endl;
        file << isValid << std::endl;
        file.close();
        std::cout << "Ticket data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

std::unique_ptr<Ticket> Ticket::loadFromFile(const std::string& filename,
                                             const std::vector<std::unique_ptr<Event>>& events,
                                             const std::vector<std::unique_ptr<User>>& users) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string id, eventName, username;
        double price;
        bool isValid;

        std::getline(file, id);
        std::getline(file, eventName);
        std::getline(file, username);
        file >> price >> isValid;

        auto eventIt = std::find_if(events.begin(), events.end(),
            [&eventName](const std::unique_ptr<Event>& e) { return e->getName() == eventName; });
        
        auto userIt = std::find_if(users.begin(), users.end(),
            [&username](const std::unique_ptr<User>& u) { return u->getUsername() == username; });

        if (eventIt != events.end() && userIt != users.end()) {
            auto ticket = std::unique_ptr<Ticket>(new Ticket(eventIt->get(), userIt->get(), price));
            ticket->id = id;
            ticket->isValid = isValid;
            file.close();
            std::cout << "Ticket data loaded from file: " << filename << std::endl;
            return ticket;
        } else {
            std::cout << "Event or User not found when loading ticket." << std::endl;
        }

        file.close();
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
    return nullptr;
}
