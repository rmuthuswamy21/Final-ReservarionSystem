#ifndef FACILITY_H
#define FACILITY_H

#include <vector>
#include <memory>
#include <chrono>

class Reservation;

class Facility {
private:
    std::vector<std::unique_ptr<Reservation>> schedule;
    const int openingHour = 8;  // 8 AM
    const int closingHour = 23; // 11 PM
    const int maxCapacity = 40;

public:
    Facility() = default;

    bool isAvailable(const std::chrono::system_clock::time_point& startTime, 
                     const std::chrono::system_clock::time_point& endTime) const;
    bool addReservation(std::unique_ptr<Reservation> reservation);
    void removeReservation(const std::string& reservationId);
    const std::vector<std::unique_ptr<Reservation>>& getReservations() const;
    void displaySchedule() const;

    // Getters for facility properties
    int getOpeningHour() const { return openingHour; }
    int getClosingHour() const { return closingHour; }
    int getMaxCapacity() const { return maxCapacity; }

    // File I/O operations
    void saveToFile(const std::string& filename) const;
    static std::unique_ptr<Facility> loadFromFile(const std::string& filename);
};

#endif // FACILITY_H