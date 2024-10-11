#ifndef RESERVATION_H
#define RESERVATION_H

#include <string>
#include <chrono>
#include <memory>
#include "LayoutStyle.h"

class User;
class FacilityManager;

class Reservation {
private:
    std::string id;
    User* user;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    LayoutStyle layoutStyle;
    bool isPublic;
    bool isPaid;
    double totalCost;

    static int nextId;

public:
    Reservation(User* user, const std::chrono::system_clock::time_point& start,
                const std::chrono::system_clock::time_point& end, LayoutStyle style, bool isPublic);

    bool overlaps(const std::chrono::system_clock::time_point& start,
                  const std::chrono::system_clock::time_point& end) const;
    double calculateCost();
    bool makePayment(double amount);
    bool cancelReservation(FacilityManager& manager);
    std::string toString() const;
    double calculateRefund() const;

    // Getters
    const std::string& getId() const { return id; }
    User* getUser() const { return user; }
    const std::chrono::system_clock::time_point& getStartTime() const { return startTime; }
    const std::chrono::system_clock::time_point& getEndTime() const { return endTime; }
    LayoutStyle getLayoutStyle() const { return layoutStyle; }
    bool getIsPublic() const { return isPublic; }
    bool getIsPaid() const { return isPaid; }
    double getTotalCost() const { return totalCost; }

    // Setter for id (used in loading from file)
    void setId(const std::string& newId) { id = newId; }

    // File operations
    void saveToFile(const std::string& filename) const;
    static std::unique_ptr<Reservation> loadFromFile(const std::string& filename, User* user);
};

#endif // RESERVATION_H