#include "Reservation.h"
#include "User.h"
#include "FacilityManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

int Reservation::nextId = 1;

Reservation::Reservation(User* user, const std::chrono::system_clock::time_point& start,
                         const std::chrono::system_clock::time_point& end, LayoutStyle style, bool isPublic)
    : user(user), startTime(start), endTime(end), layoutStyle(style), isPublic(isPublic), isPaid(false)
{
    std::stringstream ss;
    ss << "RES" << std::setw(4) << std::setfill('0') << nextId++;
    id = ss.str();

    totalCost = calculateCost();
}

bool Reservation::overlaps(const std::chrono::system_clock::time_point& start,
                           const std::chrono::system_clock::time_point& end) const
{
    return (start < endTime && end > startTime);
}

double Reservation::calculateCost()
{
    const double SERVICE_CHARGE = 10.0;
    double hourlyRate;

    switch (user->getType()) {
        case UserType::Resident:
            hourlyRate = 10.0;
            break;
        case UserType::NonResident:
            hourlyRate = 15.0;
            break;
        case UserType::City:
            hourlyRate = 5.0;
            break;
        case UserType::Organization:
            hourlyRate = 20.0;
            break;
        default:
            hourlyRate = 15.0; // Default rate
    }

    auto duration = std::chrono::duration_cast<std::chrono::hours>(endTime - startTime);
    return SERVICE_CHARGE + (hourlyRate * duration.count());
}

bool Reservation::makePayment(double amount)
{
    if (amount >= totalCost) {
        isPaid = true;
        return true;
    }
    return false;
}

bool Reservation::cancelReservation(FacilityManager& manager)
{
    auto now = std::chrono::system_clock::now();
    auto oneWeek = std::chrono::hours(24 * 7);
    
    if (startTime - now > oneWeek) {
        // Full refund
        if (isPaid) {
            manager.processRefund(totalCost);
        }
        return true;
    } else if (startTime - now > std::chrono::hours(24)) {
        // 1% penalty
        if (isPaid) {
            double penalty = totalCost * 0.01;
            double refundAmount = totalCost - penalty;
            manager.processRefund(refundAmount);
        }
        return true;
    }
    // No refund within 24 hours
    return false;
}

std::string Reservation::toString() const
{
    std::stringstream ss;
    auto timeToString = [](const std::chrono::system_clock::time_point& tp) {
        auto t = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M");
        return ss.str();
    };

    ss << "Reservation " << id << " for " << user->getUsername()
       << ": " << timeToString(startTime) << " to " << timeToString(endTime)
       << " (Layout: " << static_cast<int>(layoutStyle) << ")"
       << " - " << (isPublic ? "Public" : "Private")
       << " - " << (isPaid ? "Paid" : "Unpaid")
       << " - Cost: $" << std::fixed << std::setprecision(2) << totalCost;

    return ss.str();
}

double Reservation::calculateRefund() const
{
    auto now = std::chrono::system_clock::now();
    auto oneWeek = std::chrono::hours(24 * 7);
    
    if (startTime - now > oneWeek) {
        // Full refund
        return totalCost;
    } else if (startTime - now > std::chrono::hours(24)) {
        // 1% penalty
        double penalty = totalCost * 0.01;
        return totalCost - penalty;
    }
    // No refund within 24 hours
    return 0.0;
}

void Reservation::saveToFile(const std::string& filename) const
{
    std::ofstream file(filename);
    if (file.is_open()) {
        file << id << std::endl;
        file << user->getUsername() << std::endl;
        file << startTime.time_since_epoch().count() << std::endl;
        file << endTime.time_since_epoch().count() << std::endl;
        file << static_cast<int>(layoutStyle) << std::endl;
        file << isPublic << std::endl;
        file << isPaid << std::endl;
        file << totalCost << std::endl;
        file.close();
        std::cout << "Reservation data saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

std::unique_ptr<Reservation> Reservation::loadFromFile(const std::string& filename, User* user)
{
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string id, username;
        long long startTimeCount, endTimeCount;
        int layoutStyleInt;
        bool isPublic, isPaid;
        double totalCost;

        std::getline(file, id);
        std::getline(file, username);
        file >> startTimeCount >> endTimeCount >> layoutStyleInt >> isPublic >> isPaid >> totalCost;

        auto startTime = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(startTimeCount));
        auto endTime = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(endTimeCount));
        auto layoutStyle = static_cast<LayoutStyle>(layoutStyleInt);

        auto reservation = std::unique_ptr<Reservation>(new Reservation(user, startTime, endTime, layoutStyle, isPublic));
        reservation->setId(id);
        reservation->isPaid = isPaid;
        reservation->totalCost = totalCost;

        file.close();
        std::cout << "Reservation data loaded from file: " << filename << std::endl;
        return reservation;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
        return nullptr;
    }
}