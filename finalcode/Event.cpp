#include "Event.h"
#include "User.h"
#include "Ticket.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include "make_unique.h" // Include this if you have a custom make_unique implementation

Event::Event(const std::string& name, const std::string& description, User* organizer,
             double ticketPrice, int maxCapacity, bool isPublic,
             bool allowsResidents, bool allowsNonResidents,
             const std::chrono::system_clock::time_point& startTime,
             const std::chrono::system_clock::time_point& endTime)
    : name(name), description(description), organizer(organizer),
      ticketPrice(ticketPrice), maxCapacity(maxCapacity), currentCapacity(0),
      isPublic(isPublic), allowsResidents(allowsResidents),
      allowsNonResidents(allowsNonResidents), startTime(startTime), endTime(endTime) {}

std::unique_ptr<Ticket> Event::purchaseTicket(User* user) {
    if (isSoldOut()) {
        std::cout << "Event is sold out. Adding user to waitlist." << std::endl;
        addToWaitlist(user);
        return nullptr;
    }

    if ((user->getType() == UserType::Resident && !allowsResidents) ||
        (user->getType() == UserType::NonResident && !allowsNonResidents)) {
        std::cout << "User type not allowed for this event." << std::endl;
        return nullptr;
    }

    if (user->deductFromBudget(ticketPrice)) {
        auto ticket = std::unique_ptr<Ticket>(new Ticket(this, user, ticketPrice));
        tickets.push_back(std::unique_ptr<Ticket>(new Ticket(*ticket)));
        currentCapacity++;
        return std::unique_ptr<Ticket>(new Ticket(*tickets.back()));
    } else {
        std::cout << "Insufficient funds to purchase ticket." << std::endl;
        return nullptr;
    }
}

bool Event::cancelTicket(const Ticket* ticket) {
    auto it = std::find_if(tickets.begin(), tickets.end(),
        [ticket](const std::unique_ptr<Ticket>& t) { return t.get() == ticket; });

    if (it != tickets.end()) {
        User* user = (*it)->getUser();
        user->addToBudget(ticketPrice); // Refund the ticket price
        tickets.erase(it);
        currentCapacity--;
        processWaitlist();
        return true;
    }
    return false;
}

void Event::addToWaitlist(User* user) {
    waitlist.push(user);
}

void Event::processWaitlist() {
    while (!waitlist.empty() && !isSoldOut()) {
        User* user = waitlist.front();
        waitlist.pop();
        
        auto ticket = purchaseTicket(user);
        if (ticket) {
            std::cout << "A ticket is now available for " << user->getUsername() 
                      << " for the event: " << name << std::endl;
        }
    }
}

void Event::cancelEvent() {
    for (const auto& ticket : tickets) {
        User* user = ticket->getUser();
        user->addToBudget(ticketPrice); // Refund all tickets
        std::cout << "Event cancelled. Refund issued to " << user->getUsername() 
                  << " for the amount of $" << ticketPrice << std::endl;
    }
    tickets.clear();
    currentCapacity = 0;

    std::queue<User*> emptyQueue;
    std::swap(waitlist, emptyQueue);
}

std::string Event::toString() const {
    std::stringstream ss;
    auto timeToString = [](const std::chrono::system_clock::time_point& tp) {
        auto t = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M");
        return ss.str();
    };

    ss << "Event: " << name << "\n"
       << "Description: " << description << "\n"
       << "Organizer: " << organizer->getUsername() << "\n"
       << "Ticket Price: $" << std::fixed << std::setprecision(2) << ticketPrice << "\n"
       << "Capacity: " << currentCapacity << "/" << maxCapacity << "\n"
       << "Public Event: " << (isPublic ? "Yes" : "No") << "\n"
       << "Allows Residents: " << (allowsResidents ? "Yes" : "No") << "\n"
       << "Allows Non-Residents: " << (allowsNonResidents ? "Yes" : "No") << "\n"
       << "Start Time: " << timeToString(startTime) << "\n"
       << "End Time: " << timeToString(endTime) << "\n"
       << "Waitlist Size: " << waitlist.size();
    return ss.str();
}

void Event::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << name << std::endl;
        file << description << std::endl;
        file << organizer->getUsername() << std::endl;
        file << ticketPrice << std::endl;
        file << maxCapacity << std::endl;
        file << currentCapacity << std::endl;
        file << isPublic << std::endl;
        file << allowsResidents << std::endl;
        file << allowsNonResidents << std::endl;
        file << startTime.time_since_epoch().count() << std::endl;
        file << endTime.time_since_epoch().count() << std::endl;

        // Save tickets
        file << tickets.size() << std::endl;
        for (const auto& ticket : tickets) {
            file << ticket->getUser()->getUsername() << std::endl;
        }

        // Save waitlist
        file << waitlist.size() << std::endl;
        std::queue<User*> waitlistCopy = waitlist;
        while (!waitlistCopy.empty()) {
            file << waitlistCopy.front()->getUsername() << std::endl;
            waitlistCopy.pop();
        }

        file.close();
        std::cout << "Event data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

std::unique_ptr<Event> Event::loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<User>>& users) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string name, description, organizerUsername;
        double ticketPrice;
        int maxCapacity, currentCapacity;
        bool isPublic, allowsResidents, allowsNonResidents;
        long long startTimeCount, endTimeCount;

        std::getline(file, name);
        std::getline(file, description);
        std::getline(file, organizerUsername);
        file >> ticketPrice >> maxCapacity >> currentCapacity >> isPublic >> allowsResidents >> allowsNonResidents;
        file >> startTimeCount >> endTimeCount;

        auto organizer = std::find_if(users.begin(), users.end(),
            [&organizerUsername](const std::unique_ptr<User>& u) { return u->getUsername() == organizerUsername; });

        if (organizer == users.end()) {
            std::cout << "Organizer not found. Unable to load event." << std::endl;
            return nullptr;
        }

        auto startTime = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(startTimeCount));
        auto endTime = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(endTimeCount));

        auto event = std::unique_ptr<Event>(new Event(name, description, organizer->get(), ticketPrice, maxCapacity,
                                             isPublic, allowsResidents, allowsNonResidents, startTime, endTime));
        event->currentCapacity = currentCapacity;

        // Load tickets
        int ticketCount;
        file >> ticketCount;
        for (int i = 0; i < ticketCount; ++i) {
            std::string username;
            file >> username;
            auto user = std::find_if(users.begin(), users.end(),
                [&username](const std::unique_ptr<User>& u) { return u->getUsername() == username; });
            if (user != users.end()) {
                event->tickets.push_back(std::unique_ptr<Ticket>(new Ticket(event.get(), user->get(), ticketPrice)));
            }
        }

        // Load waitlist
        int waitlistSize;
        file >> waitlistSize;
        for (int i = 0; i < waitlistSize; ++i) {
            std::string username;
            file >> username;
            auto user = std::find_if(users.begin(), users.end(),
                [&username](const std::unique_ptr<User>& u) { return u->getUsername() == username; });
            if (user != users.end()) {
                event->waitlist.push(user->get());
            }
        }

        file.close();
        std::cout << "Event data loaded from file: " << filename << std::endl;
        return event;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
        return nullptr;
    }
}

void Event::displayAttendees() const {
    if (!isPublic) {
        std::cout << "This is a private event. Attendee list is not available." << std::endl;
        return;
    }

    std::cout << "Attendees for event '" << name << "':" << std::endl;
    for (const auto& ticket : tickets) {
        std::cout << ticket->getUser()->getUsername() << std::endl;
    }
}