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
    vector<Route> routes;
    vector<Ticket> tickets;
    int nextTicketID;
    const string BOOKING_FILE = "bookings.csv";
    const string ROUTE_FILE = "routes.csv";

    void loadRoutes() {
        ifstream file(ROUTE_FILE);
        if (!file) {
            routes.push_back(Route(1, "New Delhi", "Chennai", 6920.00, 30));
            routes.push_back(Route(2, "Lucknow", "Pune", 7200.00, 25));
            return;
        }
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, from, to, fare, cap;
            getline(ss, id, ','); getline(ss, from, ','); getline(ss, to, ',');
            getline(ss, fare, ','); getline(ss, cap, ',');
            routes.push_back(Route(stoi(id), from, to, stod(fare), stoi(cap)));
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
            
            for (auto &r : routes) {
                if (r.getID() == routeID) r.markSeat(seatNum, true);
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

public:
    ReservationSystem() : nextTicketID(1001) {
        loadRoutes();    
        loadBookings();  
    }

    void displayAllRoutes() const {
        cout << "\nAvailable Routes:\n";
        cout << left << setw(8) << "ID" << setw(15) << "From" << setw(15) << "To" 
             << setw(10) << "Fare" << setw(10) << "Seats" << endl;
        for (const auto &r : routes) r.displayRoute();
    }

    void executeBooking(string name, int rID, int sNum) {
        auto it = find_if(routes.begin(), routes.end(), [&](const Route &r) { return r.getID() == rID; });
        
        if (it == routes.end()) throw runtime_error("Invalid Route ID!");
        
        if (it->getAvailableCount() == 0) {
            throw runtime_error("CRITICAL: Route " + to_string(rID) + " is FULL. Booking aborted.");
        }

        if (!it->isSeatAvailable(sNum)) throw runtime_error("Seat " + to_string(sNum) + " is already booked.");

        it->markSeat(sNum, true);
        tickets.push_back(Ticket(nextTicketID++, name, rID, sNum, it->getFare()));
        saveData();
        cout << "Successfully Booked! Ticket ID: " << nextTicketID - 1 << " (Seat " << sNum << ")\n";
    }

    void bookTicket() {
        int rID, sNum;
        string name;
        displayAllRoutes();
        cout << "\nEnter Route ID: "; cin >> rID;
        
        // Immediate check for full route
        auto it = find_if(routes.begin(), routes.end(), [&](const Route &r) { return r.getID() == rID; });
        if (it != routes.end() && it->getAvailableCount() == 0) {
            cout << "ALERT: This route is full! No further bookings allowed.\n";
            return;
        }

        cout << "Enter Seat Number: "; cin >> sNum;
        cout << "Enter Passenger Name: "; cin.ignore(); getline(cin, name);
        try {
            executeBooking(name, rID, sNum);
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void cancelTicket(int tID) {
        auto it = find_if(tickets.begin(), tickets.end(), [&](const Ticket &t) { return t.getTicketID() == tID; });
        if (it != tickets.end()) {
            for (auto &r : routes) {
                if (r.getID() == it->getRouteID()) r.markSeat(it->getSeatNumber(), false);
            }
            tickets.erase(it);
            saveData();
            cout << "Ticket " << tID << " Cancelled Successfully.\n";
        } else {
            throw runtime_error("Ticket ID not found.");
        }
    }

    void listAllTickets() const {
        if (tickets.empty()) {
            cout << "\nNo tickets booked yet.\n";
            return;
        }
        cout << "\n--- LIST OF ALL BOOKED TICKETS ---\n";
        cout << left << setw(10) << "TID" << setw(20) << "Name" << setw(10) << "RID" << setw(10) << "Seat" << "Price" << endl;
        for (const auto &t : tickets) t.displayTicket();
        cout << "----------------------------------\n";
    }

    void generateReports() const {
        map<int, double> revenue;
        map<int, int> countMap;
        int maxBookings = 0, popularRoute = -1;

        for (const auto &t : tickets) {
            revenue[t.getRouteID()] += t.getPrice();
            countMap[t.getRouteID()]++;
            if (countMap[t.getRouteID()] > maxBookings) {
                maxBookings = countMap[t.getRouteID()];
                popularRoute = t.getRouteID();
            }
        }

        cout << "\n--- REVENUE REPORT ---" << endl;
        for (const auto &r : routes) {
            cout << "Route " << r.getID() << " (" << r.getFrom() << " -> " << r.getTo() << "): "
                 << "Tickets: " << countMap[r.getID()] << " | Revenue: ₹" << revenue[r.getID()] << endl;
        }
        if (popularRoute != -1)
            cout << "\nMost Popular Route: ID " << popularRoute << " with " << maxBookings << " bookings.\n";
    }

    void handleCLI(int argc, char* argv[]) {
        try {
            string cmd = argv[1];
            if (cmd == "list_routes") displayAllRoutes();
            else if (cmd == "list_tickets") listAllTickets();
            else if (cmd == "stats") generateReports();
            else if (cmd == "book") {
                if (argc < 5) throw invalid_argument("Usage: book <name> <rid> <seat1>...");
                string name = argv[2];
                int rid = stoi(argv[3]);
                for (int i = 4; i < argc; ++i) executeBooking(name, rid, stoi(argv[i]));
            }
            else if (cmd == "cancel") {
                if (argc < 3) throw invalid_argument("Usage: cancel <tid>");
                cancelTicket(stoi(argv[2]));
            }
            else throw invalid_argument("Unknown command: " + cmd);
        } catch (const exception& e) {
            cerr << "CLI Error: " << e.what() << endl;
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