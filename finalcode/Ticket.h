#ifndef TICKET_H
#define TICKET_H

#include <string>
#include <memory>
#include <vector>
class Event;
class User;

class Ticket {
private:
    std::string id;
    Event* event;
    User* user;
    double price;
    bool isValid;

    static int nextId;

public:
    Ticket(Event* event, User* user, double price);

    // Getters
    const std::string& getId() const { return id; }
    Event* getEvent() const { return event; }
    User* getUser() const { return user; }
    double getPrice() const { return price; }
    bool getIsValid() const { return isValid; }

    bool cancel();
    std::string toString() const;

    // File I/O operations
    void saveToFile(const std::string& filename) const;
    static std::unique_ptr<Ticket> loadFromFile(const std::string& filename, 
                                                const std::vector<std::unique_ptr<Event>>& events,
                                                const std::vector<std::unique_ptr<User>>& users);
};

#endif // TICKET_H
