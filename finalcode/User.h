#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>

class Reservation;
class Ticket;
class Event;

enum class UserType {
    Resident,
    NonResident,
    City,
    Organization
};

class User {
protected:
    std::string username;
    std::string password;
    UserType type;
    double budget;
    std::vector<std::unique_ptr<Reservation>> reservations;
    std::vector<std::unique_ptr<Ticket>> tickets;

public:
    User(const std::string& username, const std::string& password, UserType type);
    virtual ~User() = default;

    bool login(const std::string& inputUsername, const std::string& inputPassword) const;
    virtual bool makeReservation(std::unique_ptr<Reservation> reservation);
    virtual bool cancelReservation(const std::string& reservationId);
    virtual bool purchaseTicket(std::unique_ptr<Ticket> ticket);
    virtual void viewSchedule() const;
    void addToBudget(double amount);
    bool deductFromBudget(double amount);

    // Getters
    const std::string& getUsername() const { return username; }
    UserType getType() const { return type; }
    double getBudget() const { return budget; }
    const std::vector<std::unique_ptr<Reservation>>& getReservations() const { return reservations; }

    // File operations
    virtual void saveToFile(const std::string& filename) const;
    virtual void loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<Event>>& events);

    // New method
    virtual void displayUserInfo() const;
};

#endif // USER_H
