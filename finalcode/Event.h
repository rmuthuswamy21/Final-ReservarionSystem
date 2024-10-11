#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <chrono>

class User;
class Ticket;

class Event {
private:
    std::string name;
    std::string description;
    User* organizer;
    double ticketPrice;
    int maxCapacity;
    int currentCapacity;
    bool isPublic;
    bool allowsResidents;
    bool allowsNonResidents;
    std::vector<std::unique_ptr<Ticket>> tickets;
    std::queue<User*> waitlist;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;

public:
    Event(const std::string& name, const std::string& description, User* organizer,
          double ticketPrice, int maxCapacity, bool isPublic,
          bool allowsResidents, bool allowsNonResidents,
          const std::chrono::system_clock::time_point& startTime = std::chrono::system_clock::now(),
          const std::chrono::system_clock::time_point& endTime = std::chrono::system_clock::now() + std::chrono::hours(24));

    std::unique_ptr<Ticket> purchaseTicket(User* user);
    bool cancelTicket(const Ticket* ticket);
    void addToWaitlist(User* user);
    void processWaitlist();
    void cancelEvent();
    std::string toString() const;

    // Getters
    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    User* getOrganizer() const { return organizer; }
    double getTicketPrice() const { return ticketPrice; }
    int getMaxCapacity() const { return maxCapacity; }
    int getCurrentCapacity() const { return currentCapacity; }
    bool getIsPublic() const { return isPublic; }
    bool getAllowsResidents() const { return allowsResidents; }
    bool getAllowsNonResidents() const { return allowsNonResidents; }
    const std::chrono::system_clock::time_point& getStartTime() const { return startTime; }
    const std::chrono::system_clock::time_point& getEndTime() const { return endTime; }

    // File operations
    void saveToFile(const std::string& filename) const;
    static std::unique_ptr<Event> loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<User>>& users);

    // Helper methods
    bool isSoldOut() const { return currentCapacity >= maxCapacity; }
    void displayAttendees() const;
};

#endif // EVENT_H