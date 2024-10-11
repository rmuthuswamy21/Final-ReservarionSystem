#include "Facility.h"
#include "Reservation.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <chrono>

bool Facility::isAvailable(const std::chrono::system_clock::time_point& startTime, 
                           const std::chrono::system_clock::time_point& endTime) const {
    // Check if the facility is open during the requested time
    auto startHour = std::chrono::duration_cast<std::chrono::hours>(startTime.time_since_epoch()).count() % 24;
    auto endHour = std::chrono::duration_cast<std::chrono::hours>(endTime.time_since_epoch()).count() % 24;
    
    if (startHour < openingHour || endHour > closingHour || startHour >= endHour) {
        return false;
    }

    // Check for conflicts with existing reservations
    for (const auto& reservation : schedule) {
        if ((startTime < reservation->getEndTime()) && (endTime > reservation->getStartTime())) {
            return false;
        }
    }

    return true;
}

bool Facility::addReservation(std::unique_ptr<Reservation> reservation) {
    if (isAvailable(reservation->getStartTime(), reservation->getEndTime())) {
        schedule.push_back(std::move(reservation));
        return true;
    }
    return false;
}

void Facility::removeReservation(const std::string& reservationId) {
    auto it = std::find_if(schedule.begin(), schedule.end(),
        [&reservationId](const std::unique_ptr<Reservation>& r) { return r->getId() == reservationId; });
    
    if (it != schedule.end()) {
        schedule.erase(it);
    }
}

const std::vector<std::unique_ptr<Reservation>>& Facility::getReservations() const {
    return schedule;
}

void Facility::displaySchedule() const {
    std::cout << "Facility Schedule:" << std::endl;
    std::cout << "Operating Hours: " << openingHour << ":00 - " << closingHour << ":00" << std::endl;
    std::cout << "Maximum Capacity: " << maxCapacity << " guests" << std::endl;
    std::cout << std::endl;

    if (schedule.empty()) {
        std::cout << "No reservations scheduled." << std::endl;
        return;
    }

    // Sort reservations by start time
    std::vector<Reservation*> sortedSchedule;
    for (const auto& res : schedule) {
        sortedSchedule.push_back(res.get());
    }
    std::sort(sortedSchedule.begin(), sortedSchedule.end(),
        [](const Reservation* a, const Reservation* b) { return a->getStartTime() < b->getStartTime(); });

    // Display reservations
    for (const auto& reservation : sortedSchedule) {
        auto startTime = std::chrono::system_clock::to_time_t(reservation->getStartTime());
        auto endTime = std::chrono::system_clock::to_time_t(reservation->getEndTime());

        std::cout << std::put_time(std::localtime(&startTime), "%Y-%m-%d %H:%M") << " - "
                  << std::put_time(std::localtime(&endTime), "%H:%M") << " : "
                  << reservation->getUser()->getUsername()
                  << " (Layout: " << layoutStyleToString(reservation->getLayoutStyle()) << ")"
                  << (reservation->getIsPublic() ? " [Public]" : " [Private]")
                  << std::endl;
    }
}

void Facility::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << openingHour << std::endl;
        file << closingHour << std::endl;
        file << maxCapacity << std::endl;
        
        file << schedule.size() << std::endl;
        for (const auto& reservation : schedule) {
            file << reservation->getId() << std::endl;
            reservation->saveToFile(filename + "_" + reservation->getId());
        }

        file.close();
        std::cout << "Facility data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

std::unique_ptr<Facility> Facility::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        auto facility = std::unique_ptr<Facility>(new Facility());
        
        int openingHour, closingHour, maxCapacity;
        file >> openingHour >> closingHour >> maxCapacity;

        // Note: We don't set these values as they are const in the class.
        // In a real-world scenario, you might want to make these configurable.

        int scheduleSize;
        file >> scheduleSize;
        for (int i = 0; i < scheduleSize; ++i) {
            std::string reservationId;
            file >> reservationId;
            auto reservation = Reservation::loadFromFile(filename + "_" + reservationId, nullptr);
            if (reservation) {
                facility->schedule.push_back(std::move(reservation));
            }
        }

        file.close();
        std::cout << "Facility data loaded from file: " << filename << std::endl;
        return facility;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
        return nullptr;
    }
}