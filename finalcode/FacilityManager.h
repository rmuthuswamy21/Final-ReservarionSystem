#ifndef FACILITY_MANAGER_H
#define FACILITY_MANAGER_H

#include <vector>
#include <memory>
#include <string>
#include "Reservation.h"

class Facility;

class FacilityManager {
private:
    Facility& facility;
    std::vector<std::unique_ptr<Reservation>> pendingReservations;
    double budget;

public:
    FacilityManager(Facility& facility);

    bool approveReservation(const std::string& reservationId);
    bool rejectReservation(const std::string& reservationId);
    void processRefund(double amount);
    void updateBudget(double amount);
    bool checkReservationLimits(const User* user, const Reservation* newReservation) const;
    void displayPendingReservations() const;
    void displayBudget() const;
    void addPendingReservation(std::unique_ptr<Reservation> reservation);
    bool isAvailable(const std::chrono::system_clock::time_point& startTime, 
                     const std::chrono::system_clock::time_point& endTime) const;

    // File I/O operations
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<User>>& users);
};

#endif // FACILITY_MANAGER_H