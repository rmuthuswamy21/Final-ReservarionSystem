#ifndef CITYUSER_H
#define CITYUSER_H

#include "User.h"
#include <vector>
#include <memory>

class FacilityManager;
class Event;

class CityUser : public User {
private:
    FacilityManager* facilityManager;

public:
    CityUser(const std::string& username, const std::string& password);

    void setFacilityManager(FacilityManager* manager) { facilityManager = manager; }

    bool makeReservation(std::unique_ptr<Reservation> reservation) override;
    bool cancelReservation(const std::string& reservationId) override;
    bool purchaseTicket(std::unique_ptr<Ticket> ticket) override;

    void viewSchedule() const override;

    // File operations
    void saveToFile(const std::string& filename) const override;
    void loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<Event>>& events) override;

    // City-specific operations
    bool approveReservation(const std::string& reservationId);
    bool rejectReservation(const std::string& reservationId);
    void viewPendingReservations() const;
    void viewFacilityBudget() const;
};

#endif // CITYUSER_H