#ifndef ORGANIZATION_H
#define ORGANIZATION_H

#include "User.h"
#include "LayoutStyle.h"
#include <vector>
#include <memory>

class Event;

class Organization : public User {
public:
    Organization(const std::string& username, const std::string& password);

    bool makeReservation(std::unique_ptr<Reservation> reservation) override;
    bool cancelReservation(const std::string& reservationId) override;
    bool purchaseTicket(std::unique_ptr<Ticket> ticket) override;

    void viewSchedule() const override;

    void saveToFile(const std::string& filename) const override;
    void loadFromFile(const std::string& filename, const std::vector<std::unique_ptr<Event>>& events) override;

private:
    bool isValidLayoutStyle(LayoutStyle style) const;
};

#endif // ORGANIZATION_H