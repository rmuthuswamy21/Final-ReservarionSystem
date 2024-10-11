#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <iomanip>
#include "User.h"
#include "Facility.h"
#include "FacilityManager.h"
#include "Reservation.h"
#include "Ticket.h"
#include "Event.h"
#include "Resident.h"
#include "NonResident.h"
#include "CityUser.h"
#include "Organization.h"
#include "LayoutStyle.h"

// Helper function to create unique_ptr (for C++11 compatibility)
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void displayMainMenu() {
    std::cout << "\nCommunity Center Management System" << std::endl;
    std::cout << "1. Login" << std::endl;
    std::cout << "2. View Schedule" << std::endl;
    std::cout << "3. Make Reservation" << std::endl;
    std::cout << "4. Cancel Reservation" << std::endl;
    std::cout << "5. Purchase Ticket" << std::endl;
    std::cout << "6. Create Event" << std::endl;
    std::cout << "7. Facility Management" << std::endl;
    std::cout << "8. Make Payment for Reservation" << std::endl;
    std::cout << "9. View User Info" << std::endl;
    std::cout << "10. Logout" << std::endl;
    std::cout << "11. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

User* loginUser(const std::vector<std::unique_ptr<User>>& users) {
    std::cout << "\nLogin" << std::endl;
    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    for (const auto& user : users) {
        if (user->login(username, password)) {
            std::cout << "Login successful. Welcome, " << username << "!" << std::endl;
            return user.get();
        }
    }
    std::cout << "Invalid username or password. Please try again." << std::endl;
    return nullptr;
}

void displayFacilityManagementMenu(FacilityManager& manager) {
    while (true) {
        std::cout << "\nFacility Management Menu" << std::endl;
        std::cout << "1. View Pending Reservations" << std::endl;
        std::cout << "2. Approve Reservation" << std::endl;
        std::cout << "3. Reject Reservation" << std::endl;
        std::cout << "4. View Facility Budget" << std::endl;
        std::cout << "5. Return to Main Menu" << std::endl;
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1:
                manager.displayPendingReservations();
                break;
            case 2: {
                std::string reservationId;
                std::cout << "Enter the ID of the reservation to approve: ";
                std::cin >> reservationId;
                if (manager.approveReservation(reservationId)) {
                    std::cout << "Reservation approved successfully." << std::endl;
                } else {
                    std::cout << "Failed to approve reservation." << std::endl;
                }
                break;
            }
            case 3: {
                std::string reservationId;
                std::cout << "Enter the ID of the reservation to reject: ";
                std::cin >> reservationId;
                if (manager.rejectReservation(reservationId)) {
                    std::cout << "Reservation rejected successfully." << std::endl;
                } else {
                    std::cout << "Failed to reject reservation." << std::endl;
                }
                break;
            }
            case 4:
                manager.displayBudget();
                break;
            case 5:
                return;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}

std::chrono::system_clock::time_point stringToTimePoint(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void loadUsersFromFile(std::vector<std::unique_ptr<User>>& users, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password, type;
        double budget;
        
        if (!(iss >> username >> password >> type >> budget)) {
            std::cerr << "Error reading user data from file" << std::endl;
            continue;
        }

        std::unique_ptr<User> user;
        if (type == "0") {
            user = std::unique_ptr<User>(new Resident(username, password));
        } else if (type == "1") {
            user = std::unique_ptr<User>(new NonResident(username, password));
        } else if (type == "2") {
            user = std::unique_ptr<User>(new CityUser(username, password));
        } else if (type == "3") {
            user = std::unique_ptr<User>(new Organization(username, password));
        } else {
            std::cerr << "Unknown user type: " << type << std::endl;
            continue;
        }

        user->addToBudget(budget);
        users.push_back(std::move(user));
    }

    file.close();
}

void loadEventsFromFile(std::vector<std::unique_ptr<Event>>& events, const std::vector<std::unique_ptr<User>>& users, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name, description, organizerUsername;
        double ticketPrice;
        int maxCapacity;
        bool isPublic, allowsResidents, allowsNonResidents;

        // Read the event name
        std::getline(iss, name, ' ');
        
        // Read the description (enclosed in quotes)
        std::getline(iss, description, '"');  // Skip leading quote
        std::getline(iss, description, '"');  // Read the actual description

        // Read the rest of the values
        iss >> organizerUsername >> ticketPrice >> maxCapacity >> isPublic >> allowsResidents >> allowsNonResidents;

        if (iss.fail()) {
            std::cerr << "Error reading event data from file" << std::endl;
            continue;
        }

        // Find the organizer in the list of users
        User* organizer = nullptr;
        for (const auto& user : users) {
            if (user->getUsername() == organizerUsername) {
                organizer = user.get();
                break;
            }
        }

        if (!organizer) {
            std::cerr << "Organizer not found: " << organizerUsername << std::endl;
            continue;
        }

        // Assuming default or placeholder values for startTime and endTime since they are not provided in the events.txt
        auto startTime = stringToTimePoint("2023-01-01T00:00:00Z");
        auto endTime = stringToTimePoint("2023-01-01T01:00:00Z");

        // Create and add the event to the list
        events.push_back(make_unique<Event>(name, description, organizer, ticketPrice, maxCapacity, 
                                                isPublic, allowsResidents, allowsNonResidents, 
                                                startTime, endTime));
    }

    file.close();
}
       

int main() {
    Facility facility;
    FacilityManager manager(facility);
    std::vector<std::unique_ptr<User>> users;
    std::vector<std::unique_ptr<Event>> events;

    // Load users and events from files
    loadUsersFromFile(users, "users.txt");
    loadEventsFromFile(events, users, "events.txt");

    User* currentUser = nullptr;

    while (true) {
        displayMainMenu();
        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1: {
                currentUser = loginUser(users);
                break;
            }
            case 2: {
                if (currentUser != nullptr) {
                    currentUser->viewSchedule();
                    facility.displaySchedule();
                } else {
                    std::cout << "Please login first." << std::endl;
                }
                break;
            }
            case 3: {
                if (currentUser != nullptr) {
                    std::cout << "Enter reservation details:" << std::endl;
                    std::string startTimeStr, endTimeStr;
                    int layoutStyleInt;
                    bool isPublic;

                    std::cout << "Start time (YYYY-MM-DD HH:MM): ";
                    std::cin.ignore();
                    std::getline(std::cin, startTimeStr);
                    std::cout << "End time (YYYY-MM-DD HH:MM): ";
                    std::getline(std::cin, endTimeStr);

                    std::cout << "Layout style (0: Meeting, 1: Lecture, 2: Wedding, 3: Dance): ";
                    std::cin >> layoutStyleInt;

                    std::cout << "Is public event? (0: No, 1: Yes): ";
                    std::cin >> isPublic;

                    auto startTime = stringToTimePoint(startTimeStr);
                    auto endTime = stringToTimePoint(endTimeStr);

                    LayoutStyle layoutStyle = static_cast<LayoutStyle>(layoutStyleInt);

                    auto reservation = make_unique<Reservation>(currentUser, startTime, endTime, layoutStyle, isPublic);
                    
                    std::cout << "The total cost for this reservation is: $" << reservation->getTotalCost() << std::endl;
                    std::cout << "Do you want to make the payment now? (1: Yes, 0: No): ";
                    bool makePaymentNow;
                    std::cin >> makePaymentNow;

                    if (makePaymentNow) {
                        double paymentAmount;
                        std::cout << "Enter the payment amount: $";
                        std::cin >> paymentAmount;
                        if (reservation->makePayment(paymentAmount)) {
                            std::cout << "Payment successful." << std::endl;
                        } else {
                            std::cout << "Payment failed. The entered amount is less than the total cost." << std::endl;
                        }
                    }

                    if (currentUser->makeReservation(std::move(reservation))) {
                        manager.addPendingReservation(std::move(reservation));
                        std::cout << "Reservation request submitted. ";
                        if (reservation->getIsPaid()) {
                            std::cout << "Waiting for approval." << std::endl;
                        } else {
                            std::cout << "Please note that the reservation will not be approved until payment is made." << std::endl;
                        }
                    } else {
                        std::cout << "Failed to make reservation." << std::endl;
                    }
                } else {
                    std::cout << "Please login first." << std::endl;
                }
                break;
            }
            case 4: {
                if (currentUser != nullptr) {
                    currentUser->viewSchedule();
                    std::string reservationId;
                    std::cout << "Enter the ID of the reservation to cancel: ";
                    std::cin >> reservationId;

                    if (currentUser->cancelReservation(reservationId)) {
                        std::cout << "Reservation canceled successfully." << std::endl;
                    } else {
                        std::cout << "Failed to cancel reservation." << std::endl;
                    }
                } else {
                    std::cout << "Please login first." << std::endl;
                }
                break;
            }
            case 5: {
                if (currentUser != nullptr) {
                    if (events.empty()) {
                        std::cout << "No events available." << std::endl;
                        break;
                    }

                    std::cout << "Available events:" << std::endl;
                    for (size_t i = 0; i < events.size(); ++i) {
                        std::cout << i << ". " << events[i]->getName() << std::endl;
                    }

                    int eventIndex;
                    std::cout << "Enter the index of the event to purchase a ticket: ";
                    std::cin >> eventIndex;

                    if (eventIndex >= 0 && eventIndex < events.size()) {
                        Event* selectedEvent = events[eventIndex].get();
                        auto ticket = selectedEvent->purchaseTicket(currentUser);
                        if (ticket) {
                            if (currentUser->purchaseTicket(std::move(ticket))) {
                                std::cout << "Ticket purchased successfully." << std::endl;
                            } else {
                                std::cout << "Failed to purchase ticket. Insufficient funds." << std::endl;
                                selectedEvent->cancelTicket(ticket.get());
                            }
                        } else {
                            std::cout << "Failed to purchase ticket. Event might be full." << std::endl;
                            std::cout << "Do you want to be added to the waitlist? (1: Yes, 0: No): ";
                            bool addToWaitlist;
                            std::cin >> addToWaitlist;
                            if (addToWaitlist) {
                                selectedEvent->addToWaitlist(currentUser);
                                std::cout << "Added to the waitlist." << std::endl;
                            }
                        }
                    } else {
                        std::cout << "Invalid event index." << std::endl;
                    }
                } else {
                    std::cout << "Please login first." << std::endl;
                }
                break;
            }
            case 6: {
                if (currentUser != nullptr) {
                    std::string name, description, startTimeStr, endTimeStr;
                    double ticketPrice;
                    int maxCapacity;
                    bool isPublic, allowsResidents, allowsNonResidents;

                    std::cout << "Enter event details:" << std::endl;
                    std::cout << "Name: ";
                    std::cin.ignore();
                    std::getline(std::cin, name);
                    std::cout << "Description: ";
                    std::getline(std::cin, description);
                    std::cout << "Ticket price: ";
                    std::cin >> ticketPrice;
                    std::cout << "Max capacity: ";
                    std::cin >> maxCapacity;
                    std::cout << "Is public event? (0: No, 1: Yes): ";
                    std::cin >> isPublic;
                    std::cout << "Allows residents? (0: No, 1: Yes): ";
                    std::cin >> allowsResidents;
                    std::cout << "Allows non-residents? (0: No, 1: Yes): ";
                    std::cin >> allowsNonResidents;
                    std::cout << "Start time (YYYY-MM-DD HH:MM): ";
                    std::cin.ignore();
                    std::getline(std::cin, startTimeStr);
                    std::cout << "End time (YYYY-MM-DD HH:MM): ";
                    std::getline(std::cin, endTimeStr);

                    auto startTime = stringToTimePoint(startTimeStr);
                    auto endTime = stringToTimePoint(endTimeStr);

                    events.push_back(make_unique<Event>(name, description, currentUser, ticketPrice, maxCapacity, 
                                                        isPublic, allowsResidents, allowsNonResidents, 
                                                        startTime, endTime));
                    std::cout << "Event created successfully." << std::endl;
                } else {
                    std::cout << "Please login first." << std::endl;
                }
                break;
            }
            case 7: {
                if (currentUser != nullptr && dynamic_cast<CityUser*>(currentUser) != nullptr) {
                    displayFacilityManagementMenu(manager);
                } else {
                    std::cout << "You don't have permission to access facility management." << std::endl;
                }
                break;
            }
            case 8: {
                if (currentUser != nullptr) {
                    currentUser->viewSchedule();
                    std::string reservationId;
                    std::cout << "Enter the ID of the reservation to pay for: ";
                    std::cin >> reservationId;

                    Reservation* reservationToPay = nullptr;
                    for (const auto& res : currentUser->getReservations()) {
                        if (res->getId() == reservationId && !res->getIsPaid()) {
                            reservationToPay = res.get();
                            break;
                        }
                    }

                    if (reservationToPay) {
                        std::cout << "The total cost for this reservation is: $" << reservationToPay->getTotalCost() << std::endl;
                        double paymentAmount;
                        std::cout << "Enter the payment amount: $";
                        std::cin >> paymentAmount;
                        if (currentUser->deductFromBudget(paymentAmount)) {
                            if (reservationToPay->makePayment(paymentAmount)) {
                                std::cout << "Payment successful." << std::endl;
                            } else {
                                std::cout << "Payment failed. The entered amount is less than the total cost." << std::endl;
                                currentUser->addToBudget(paymentAmount); // Refund the deducted amount
                            }
                        } else {
                            std::cout << "Insufficient funds in your budget." << std::endl;
                        }
                    } else {
                        std::cout << "Reservation not found or already paid." << std::endl;
                    }
                } else {
                    std::cout << "Please login first." << std::endl;
                }
                break;
            }
            case 9: {
                if (currentUser != nullptr) {
                    std::cout << "User Information:" << std::endl;
                    std::cout << "Username: " << currentUser->getUsername() << std::endl;
                    std::cout << "User Type: ";
                    switch (currentUser->getType()) {
                        case UserType::Resident:
                            std::cout << "Resident";
                            break;
                        case UserType::NonResident:
                            std::cout << "Non-Resident";
                            break;
                        case UserType::City:
                            std::cout << "City User";
                            break;
                        case UserType::Organization:
                            std::cout << "Organization";
                            break;
                    }
                    std::cout << std::endl;
                    std::cout << "Budget: $" << std::fixed << std::setprecision(2) << currentUser->getBudget() << std::endl;
                    currentUser->viewSchedule();
                } else {
                    std::cout << "Please login first." << std::endl;
                }
                break;
            }
            case 10: {
                if (currentUser != nullptr) {
                    std::cout << "Logging out. Goodbye, " << currentUser->getUsername() << "!" << std::endl;
                    currentUser = nullptr;
                } else {
                    std::cout << "No user is currently logged in." << std::endl;
                }
                break;
            }
            case 11: {
                std::cout << "Exiting the program. Goodbye!" << std::endl;
                return 0;
            }
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }

    return 0;
}
