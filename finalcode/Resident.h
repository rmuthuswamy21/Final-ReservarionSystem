#ifndef RESIDENT_H
#define RESIDENT_H

#include "User.h"

class Resident : public User {
public:
    Resident(const std::string& username, const std::string& password);

    bool makeReservation(std::unique_ptr<Reservation> reservation) override;
    bool cancelReservation(const std::string& reservationId) override;
    bool purchaseTicket(std::unique_ptr<Ticket> ticket) override;

    void viewSchedule() const override;

    // File operations
    void saveToFile(const std::string& filename) const override;
    void loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<Event>>& events) override;
};

#endif // RESIDENT_H