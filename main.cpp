#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

using namespace std;

// Seat class to represent each seat on the airplane
class Seat {
public:
    int number;
    char letter;
    bool available;
    double price;
    int row;

    Seat() : number(0), letter('A'), available(true), price(0.0) {}

    Seat(int num, char let, int r, double price) : number(num), letter(let), row(r), price(price), available(true) {}

    void book() {
        available = false;
    }

    bool isAvailable() const {
        return available;
    }

    void free() {
        available = true;
    }
};

// Ticket class
class Ticket {
public:
    int ticketID;
    string passengerName;
    string flightNumber;
    string flightDate;
    Seat seat;

    Ticket() : ticketID(0), passengerName(""), flightNumber(""), flightDate(""), seat() {}

    Ticket(int id,const string& passenger, const string& flight, const string& date, const Seat& s)
        : ticketID(id),passengerName(passenger), flightNumber(flight), flightDate(date), seat(s) {}

    void viewTicket() const {
        cout << "Ticket ID: " << ticketID << ", Passenger: " << passengerName
             << ", Flight: " << flightNumber << ", Date: " << flightDate
             << ", Seat: " << seat.number << ", Price: $" << seat.price << endl;
    }
};

// Passenger class
class Passenger {
public:
    string name;
    double balance;
    vector<Ticket> tickets; // list of tickets

    Passenger(const string& passengerName, double initialBalance = 0.0) :
    name(passengerName), balance(initialBalance) {}

    // Adds a ticket to the passenger's collection
    void addTicket(const Ticket& ticket) {
        tickets.push_back(ticket);
    }

    // Show all tickets booked by the passenger
    void showTickets() const {
        cout << "Tickets for " << name << ":\n";
        for (const auto& ticket : tickets) {
            ticket.viewTicket();  // Calls the Ticket class method to display ticket details
        }
    }

    bool findTicket(int ticketID, Ticket& outTicket) const {
        for (const auto& ticket : tickets) {
            if (ticket.ticketID == ticketID) {
                outTicket = ticket;
                return true;
            }
        }
        return false;
    }

    bool returnTicket(int ticketID) {
        for (auto it = tickets.begin(); it != tickets.end(); ++it) {
            if (it->ticketID == ticketID) {
                tickets.erase(it);
                return true;
            }
        }
        return false;
    }
    // Refund money to the passenger
    void refundMoney(double amount) {
        balance += amount;
        cout << "Refunded $" << amount << " to " << name << ". New balance: $" << balance << endl;
    }
};

class Airplane {
public:
    string flightNumber;
    string date;
    int seatsPerRow;
    map<string, Seat> seats;

    Airplane(const string& flightNum, const string& d, int seatsRow, const vector<Seat>& seatList)
        : flightNumber(flightNum), date(d), seatsPerRow(seatsRow) {
        for (const auto& seat : seatList) {
            seats[to_string(seat.number) + seat.letter] = seat;
        }
    }

    void addSeat(int seatNumber, char seatLetter, int row, double price) {
        string seatKey = to_string(seatNumber) + seatLetter; // Creates a unique seat identifier
        seats[seatKey] = Seat(seatNumber, seatLetter, row, price); // Now passes the row parameter
    }

    bool isSeatAvailable(int row, char letter) const{
        string seatKey = to_string(row) + letter; // Create a string key
        auto it = seats.find(seatKey);
        if (it != seats.end()) {
            return it->second.isAvailable(); // Check if the seat is available
        }
        return false;
    }

    bool bookSeat(int row, char letter) {
        string seatKey = to_string(row) + letter;
        auto it = seats.find(seatKey);
        if (it != seats.end() && it->second.available) {
            it->second.book();
            return true;
        }
        return false;
    }

    void returnSeat(int seatNumber, char seatLetter) {
        string seatKey = to_string(seatNumber) + seatLetter; // Create string key
        auto it = seats.find(seatKey);
        if (it != seats.end()) {
            it->second.free();
        }
    }

    void displayAvailableSeats() const {
        for (const auto& pair : seats) {
            if (isSeatAvailable(pair.second.number, pair.second.letter)) { // Use string key
                cout << "Seat " << pair.second.number << pair.second.letter << " is available at price $" << pair.second.price << endl;
            }
        }
    }

};

class File {
public:
    // Constructor opens the file
    File(const char* filename, int flags) {
        fileDescriptor = open(filename, flags);
        if (fileDescriptor == -1) {
            throw std::runtime_error("Failed to open file");
        }
    }

    // Destructor closes the file
    ~File() {
        if (fileDescriptor != -1) {
            close(fileDescriptor);
        }
    }

    // Disable copy semantics to avoid double-close
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    // Enable move semantics for transferring ownership
    File(File&& other) noexcept : fileDescriptor(other.fileDescriptor) {
        other.fileDescriptor = -1; // Nullify the moved-from object's file descriptor
    }

    File& operator=(File&& other) noexcept {
        if (this != &other) {
            if (fileDescriptor != -1) {
                close(fileDescriptor);
            }
            fileDescriptor = other.fileDescriptor;
            other.fileDescriptor = -1;
        }
        return *this;
    }

    // Reading from the file
    ssize_t read(void* buffer, size_t size) {
        return ::read(fileDescriptor, buffer, size);
    }

    // Writing to the file
    ssize_t write(const void* buffer, size_t size) {
        return ::write(fileDescriptor, buffer, size);
    }

    // File descriptor accessor
    int getFileDescriptor() const {
        return fileDescriptor;
    }

private:
    int fileDescriptor;
};

class ConfigReader {
public:
    vector<Airplane> loadConfig(const string& configFile) {
        vector<Airplane> airplanes;
        File file(configFile.c_str(), O_RDONLY);
        char buffer[4096];
        ssize_t bytesRead;
        string fileContent;
        // ifstream file(configFile);
        // if (!file.is_open()) {
        //     cout << "Unable to open configuration file." << endl;
        //     return airplanes;
        // }

        while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
            fileContent.append(buffer, bytesRead);
        }

        // Handle case where file couldn't be read
        if (bytesRead == -1) {
            throw std::runtime_error("Error reading file");
        }
        stringstream ss(fileContent);
        string line;
        while (getline(ss, line)) {
            stringstream lineStream(line);
            string date, flightNumber;
            int seatsPerRow;
            lineStream >> date >> flightNumber >> seatsPerRow;

            vector<Seat> seats;
            int seatNumber = 1;
            int rowStart, rowEnd;
            string priceStr;
            double price;

            while (lineStream >> rowStart >> rowEnd >> priceStr) {
                char dash;
                lineStream >> dash >> rowEnd >> priceStr;

                priceStr.erase(remove(priceStr.begin(), priceStr.end(), '$'), priceStr.end());
                price = stod(priceStr);

                for (int row = rowStart; row <= rowEnd; ++row) {
                    for (char letter = 'A'; letter < 'A' + seatsPerRow; ++letter) {
                        seats.push_back(Seat(seatNumber++, letter, row, price));
                    }
                }
            }

            // Add the airplane to the list with flight number, date, and seats
            airplanes.push_back(Airplane(flightNumber, date, seatsPerRow, seats));
        }

        return airplanes;
    }
};

class Program {
private:
    vector<Airplane> airplanes;
    vector<Passenger> passengers;
    vector<Ticket> tickets;

public:
    Program(const string& configFile) {
        ConfigReader configReader;
        airplanes = configReader.loadConfig(configFile);
    }

    // Find a passenger by name
    Passenger* findPassenger(const string& name) {
        for (auto& passenger : passengers) {
            if (passenger.name == name) return &passenger;
        }
        return nullptr;
    }

    // Add a new passenger
    void addPassenger(const string& name, double money) {
        passengers.push_back(Passenger(name, money));
    }

    // Book a ticket for a passenger
    void bookTicket(const string& flightNumber, const string& date, const string& seatNumber, char seatLetter, const string& passengerName) {
        for (auto& airplane : airplanes) {
            if (airplane.flightNumber == flightNumber && airplane.date == date) {
                if (airplane.bookSeat(stoi(seatNumber), seatLetter)) {
                    Passenger* passenger = findPassenger(passengerName);
                    if (!passenger) {
                        passengers.push_back(Passenger(passengerName));
                        passenger = &passengers.back();
                    }

                    int ticketID = rand(); // Generate a random ticket ID
                    auto seat = airplane.seats.find(to_string(stoi(seatNumber)) + seatLetter)->second;
                    Ticket ticket(ticketID, passengerName, flightNumber, date, seat);
                    passenger->addTicket(ticket);
                    tickets.push_back(ticket);

                    cout << "Ticket booked successfully. Ticket ID: " << ticketID << endl;
                } else {
                    cout << "Seat is unavailable or invalid.\n";
                }
                return;
            }
        }
        cout << "Flight not found.\n";
    }


    void checkAvailability(const string& flightNumber, const string& date) {
        for (const auto& airplane : airplanes) {
            if (airplane.flightNumber == flightNumber && airplane.date == date) {
                cout << "Available seats for flight " << flightNumber << " on " << date << ":\n";
                airplane.displayAvailableSeats();
                return;
            }
        }
        cout << "Flight not found.\n";
    }

    void returnTicket(int ticketID) {
        Ticket foundTicket;
        Passenger* ticketOwner = nullptr;

        // Find the passenger who owns the ticket
        for (auto& passenger : passengers) {
            if (passenger.findTicket(ticketID, foundTicket)) {
                ticketOwner = &passenger;
                break;
            }
        }


        if (ticketOwner) {
            // Find the corresponding airplane and return the seat
            for (auto& airplane : airplanes) {
                if (airplane.flightNumber == foundTicket.flightNumber && airplane.date == foundTicket.flightDate) {
                    airplane.returnSeat(foundTicket.seat.number, foundTicket.seat.letter);  // Return the seat in the airplane
                    ticketOwner->returnTicket(ticketID);           // Remove the ticket from the passenger
                    ticketOwner->refundMoney(foundTicket.seat.price);   // Refund the ticket price to the passenger
                    cout << "Ticket returned successfully. Refund issued for $" << foundTicket.seat.price << endl;
                    return;
                }
            }
        } else {
            cout << "Ticket not found.\n";
        }
    }

    // View all tickets for a passenger
    void viewBookedTickets(const string& passengerName) {
        Passenger* passenger = findPassenger(passengerName);
        if (passenger) {
            passenger->showTickets();
        } else {
            cout << "Passenger not found!\n";
        }
    }

    void viewTicket(int ticketID) {
        for (const auto& ticket : tickets) {
            if (ticket.ticketID == ticketID) {
                ticket.viewTicket();
                return;
            }
        }
        cout << "Ticket ID not found.\n";
    }

    void viewByUsername(const string& username) {
        Passenger* passenger = findPassenger(username);
        if (passenger) {
            passenger->showTickets();
        } else {
            cout << "Passenger not found.\n";
        }
    }

    void viewByFlight(const string& date, const string& flightNumber) {
        for (const auto& airplane : airplanes) {
            if (airplane.flightNumber == flightNumber && airplane.date == date) {
                cout << "Available tickets for flight " << flightNumber << " on " << date << ":\n";
                airplane.displayAvailableSeats();
                return;
            }
        }
        cout << "Flight not found.\n";
    }
};

class InputReader {
    public:
        void processInput(const string& input, Program& program) {
            istringstream iss(input);
            string command;
            iss >> command;

            if (command == "book") {
                string date, flightNumber, seatStr, passengerName;
                iss >> date >> flightNumber >> seatStr;

                string seatID = seatStr;

                size_t pos = seatStr.find_first_not_of("0123456789");
                string seatNumber = seatStr.substr(0, pos);
                char seatLetter = seatStr[pos];

                // Assuming passenger name is the remaining part of the string
                getline(iss, passengerName);
                passengerName.erase(0, passengerName.find_first_not_of(" "));

                program.bookTicket(flightNumber, date, seatNumber, seatLetter, passengerName);
            }
            else if (command == "check") {
                string date, flightNumber;
                iss >> date >> flightNumber;
                program.checkAvailability(flightNumber, date);
            }
            else if (command == "return") {
                int ticketID;
                iss >> ticketID;
                program.returnTicket(ticketID);
            }
            else if (command == "view") {
                string viewType;
                iss >> viewType;
                if (viewType == "ID") {
                    int ticketID;
                    iss >> ticketID;
                    program.viewTicket(ticketID);
                } else if (viewType == "username") {
                    string username;
                    iss >> username;
                    program.viewByUsername(username);
                } else if (viewType == "flight") {
                    string date, flightNumber;
                    iss >> date >> flightNumber;
                    program.viewByFlight(date, flightNumber);
                }
            }
        }
    };


int main() {
    Program program("/Users/yelyzaveta/CLionProjects/oop_airflight/oop_airfligth/config.txt");
    InputReader inputReader;

    string input;
    while (true) {
        cout << "Enter a command (check, book, return, view, exit): ";
        getline(cin, input);

        if (input == "exit") {
            break;
        }

        inputReader.processInput(input, program);
    }

    return 0;
};

