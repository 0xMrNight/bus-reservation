#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>

using namespace std;

// Handles seat management and route details
class Route {
private:
    int routeID;
    string from;
    string to;
    double fare;
    int capacity;
    vector<bool> seats; 

public:
    Route(int id, string f, string t, double fr, int cap = 30)
        : routeID(id), from(f), to(t), fare(fr), capacity(cap) {
        seats.assign(capacity, false);
    }

    int getID() const { return routeID; }
    string getFrom() const { return from; }
    string getTo() const { return to; }
    double getFare() const { return fare; }
    int getCapacity() const { return capacity; }

    bool isSeatAvailable(int seatNum) const {
        if (seatNum < 1 || seatNum > capacity) return false;
        return !seats[seatNum - 1];
    }

    void markSeat(int seatNum, bool status) {
        if (seatNum >= 1 && seatNum <= capacity) {
            seats[seatNum - 1] = status;
        }
    }

    int getAvailableCount() const {
        return count(seats.begin(), seats.end(), false);
    }

    int getBookedCount() const {
        return count(seats.begin(), seats.end(), true);
    }

    void displayRoute() const {
        cout << left << setw(8) << routeID << setw(15) << from << setw(15) << to 
            << setw(10) << fare << setw(10) << (to_string(getAvailableCount()) + "/" + to_string(capacity)) << endl;
    }
};

// Represents a passenger booking
class Ticket {
private:
    int ticketID;
    string passengerName;
    int routeID;
    int seatNumber;
    double price;

public:
    Ticket(int tID, string name, int rID, int sNum, double pr)
        : ticketID(tID), passengerName(name), routeID(rID), seatNumber(sNum), price(pr) {}

    int getTicketID() const { return ticketID; }
    int getRouteID() const { return routeID; }
    int getSeatNumber() const { return seatNumber; }
    string getPassengerName() const { return passengerName; }
    double getPrice() const { return price; }

    void displayTicket() const {
        cout << left << setw(10) << ticketID << setw(20) << passengerName 
             << setw(10) << routeID << setw(10) << seatNumber 
             << "₹" << fixed << setprecision(2) << price << endl;
    }
};

class ReservationSystem {
private:
    map<int, Route> routes;
    vector<Ticket> tickets;
    int nextTicketID;
    const string BOOKING_FILE = "bookings.csv";
    const string ROUTE_FILE = "routes.csv";

    void loadRoutes() {
        ifstream file(ROUTE_FILE);
        if (!file) {
            routes.emplace(1501, Route(1501, "Mumbai", "Dubai", 27400.00, 30));
            routes.emplace(1502, Route(1502, "Chennai", "Singapore", 26300.00, 40));
            return;
        }
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, from, to, fare, cap;
            getline(ss, id, ','); getline(ss, from, ','); getline(ss, to, ',');
            getline(ss, fare, ','); getline(ss, cap, ',');
            int rID = stoi(id);
            routes.emplace(rID, Route(rID, from, to, stod(fare), stoi(cap)));
        }
        file.close();
    }

    void loadBookings() {
        ifstream file(BOOKING_FILE);
        if (!file) return;
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string tID, name, rID, sNum, price;
            getline(ss, tID, ','); getline(ss, name, ','); getline(ss, rID, ',');
            getline(ss, sNum, ','); getline(ss, price, ',');

            int ticketID = stoi(tID);
            int routeID = stoi(rID);
            int seatNum = stoi(sNum);
            
            tickets.push_back(Ticket(ticketID, name, routeID, seatNum, stod(price)));
            
            // Sync seat map with existing bookings
            if (routes.count(routeID)) {
                routes.at(routeID).markSeat(seatNum, true);
            }
            if (ticketID >= nextTicketID) nextTicketID = ticketID + 1;
        }
        file.close();
    }

    void saveData() {
        ofstream file(BOOKING_FILE);
        for (const auto &t : tickets) {
            file << t.getTicketID() << "," << t.getPassengerName() << "," 
                 << t.getRouteID() << "," << t.getSeatNumber() << "," << t.getPrice() << "\n";
        }
        file.close();
    }

    void validateInput(const string& name) {
        if (name.find(',') != string::npos) {
            throw runtime_error("Invalid Input: Names cannot contain commas.");
        }
    }

public:
    ReservationSystem() : nextTicketID(1001) {
        loadRoutes();    
        loadBookings();  
    }

    void executeMultiBooking(string name, int rID, vector<int> seatNums) {
        validateInput(name);

        if (routes.find(rID) == routes.end()) {
            throw runtime_error("Route ID " + to_string(rID) + " not found.");
        }
        
        Route &selectedRoute = routes.at(rID);

        // check availability for all requested seats
        for (int sNum : seatNums) {
            if (!selectedRoute.isSeatAvailable(sNum)) {
                throw runtime_error("Seat " + to_string(sNum) + " is already occupied or invalid.");
            }
        }

        for (int sNum : seatNums) {
            selectedRoute.markSeat(sNum, true);
            tickets.push_back(Ticket(nextTicketID++, name, rID, sNum, selectedRoute.getFare()));
        }
        
        saveData();
        cout << "SUCCESS|" << (nextTicketID - seatNums.size()) << "|" << name << endl;
    }

    void displayAllRoutes() const {
        cout << "\nAvailable Routes:\n";
        cout << left << setw(8) << "ID" << setw(15) << "From" << setw(15) << "To" 
             << setw(10) << "Fare" << setw(10) << "Seats" << endl;
        for (auto const& [id, r] : routes) {
            r.displayRoute();
        }
    }

    void bookTicket() {
        int rID;
        string name, seatInput;
        displayAllRoutes();
        cout << "\nEnter Route ID: "; cin >> rID;
        cout << "Enter Passenger Name: "; cin.ignore(); getline(cin, name);
        cout << "Enter Seat Numbers (separated by space): ";
        getline(cin, seatInput);

        stringstream ss(seatInput);
        vector<int> seatNums;
        int s;
        while (ss >> s) seatNums.push_back(s);

        try {
            executeMultiBooking(name, rID, seatNums);
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void cancelTicket(int tID) {
        auto it = find_if(tickets.begin(), tickets.end(), [&](const Ticket &t) { 
            return t.getTicketID() == tID; 
        });

        if (it != tickets.end()) {
            int rID = it->getRouteID();
            if (routes.count(rID)) {
                routes.at(rID).markSeat(it->getSeatNumber(), false);
            }
            tickets.erase(it);
            saveData();
            cout << "SUCCESS|Ticket " << tID << " Cancelled Successfully.\n";
        } else {
            throw runtime_error("Error: Ticket ID " + to_string(tID) + " does not exist.");
        }
    }

    void listAllTickets() const {
        if (tickets.empty()) {
            cout << "\nNo active bookings found.\n";
            return;
        }
        cout << "\n--- CURRENT BOOKINGS ---\n";
        cout << left << setw(10) << "TID" << setw(20) << "Name" << setw(10) << "RID" << setw(10) << "Seat" << "Price" << endl;
        for (const auto &t : tickets) t.displayTicket();
    }

    void generateReports() const {
        map<int, double> revenue;
        map<int, int> countMap;
        int maxBookings = 0, popularRouteID = -1;

        for (const auto &t : tickets) {
            revenue[t.getRouteID()] += t.getPrice();
            countMap[t.getRouteID()]++;
            if (countMap[t.getRouteID()] > maxBookings) {
                maxBookings = countMap[t.getRouteID()];
                popularRouteID = t.getRouteID();
            }
        }

        cout << "\n--- REVENUE REPORT ---" << endl;
        for (auto const& [id, r] : routes) {
            cout << "Route " << id << " (" << r.getFrom() << " -> " << r.getTo() << "): "
                 << "Tickets: " << countMap[id] << " | Revenue: ₹" << fixed << setprecision(2) << revenue[id] << endl;
        }
        
        if (popularRouteID != -1) {
            cout << "\nMost Popular Route: ID " << popularRouteID << " (" << maxBookings << " bookings)\n";
        }
    }

    void handleCLI(int argc, char* argv[]) {
        try {
            if (argc < 2) return;
            string cmd = argv[1];

            if (cmd == "list_routes") displayAllRoutes();
            else if (cmd == "list_tickets") listAllTickets();
            else if (cmd == "stats") generateReports();
            else if (cmd == "book") {
                if (argc < 5) throw invalid_argument("Usage: book <name> <rid> <s1> <s2>...");
                string name = argv[2];
                int rid = stoi(argv[3]);
                vector<int> seats;
                for (int i = 4; i < argc; ++i) seats.push_back(stoi(argv[i]));
                executeMultiBooking(name, rid, seats);
            }
            else if (cmd == "cancel") {
                if (argc < 3) throw invalid_argument("Usage: cancel <tid>");
                cancelTicket(stoi(argv[2]));
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
};

int main(int argc, char* argv[]) {
    ReservationSystem sys;
    if (argc > 1) {
        sys.handleCLI(argc, argv);
        return 0;
    }

    int choice;
    do {
        cout << "\n=== BUS RESERVATION SYSTEM ===\n";
        cout << "1. Display Routes\n2. Book Ticket\n3. Cancel Ticket\n4. List All Booked Tickets\n5. Revenue Report\n6. Exit\n";
        cout << "Choice: ";
        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(1000, '\n'); continue;
        }
        switch (choice) {
            case 1: sys.displayAllRoutes(); break;
            case 2: sys.bookTicket(); break;
            case 3: {
                int id; cout << "Enter Ticket ID: "; cin >> id;
                try { sys.cancelTicket(id); } catch (const exception& e) { cout << e.what() << endl; }
                break;
            }
            case 4: sys.listAllTickets(); break;
            case 5: sys.generateReports(); break;
            case 6: cout << "Exiting...\n"; break;
            default: cout << "Invalid selection.\n";
        }
    } while (choice != 6);
    return 0;
}