#include <iostream>
#include <vector>
#include <string>
#include <fstream> // for working with file

using namespace std;

class Airplane {
private:
    int numberOfSeats;

    vector<bool> seatStatus;
    string flightInfo;
public:
    Airplane(int seats, string flightInfo) : numberOfSeats(seats), flightInfo(flightInfo) {
        seatStatus = vector<bool>(numberOfSeats, true);
    }

    // Method to book a seat
    bool bookSeat(int seatNumber) {
        if (seatNumber >= 0 && seatNumber < numberOfSeats && seatStatus[seatNumber]) {
            seatStatus[seatNumber] = false;
            return true;
        }
        return false;
    }

    bool cancelSeat(int seatNumber) {
        if (seatNumber >= 0 && seatNumber < numberOfSeats && !seatStatus[seatNumber]) {
            seatStatus[seatNumber] = true;
            return true;
        }
        return false;
    }

    bool isSeatAvailable(int seatNumber) const {
        if (seatNumber >= 0 && seatNumber < numberOfSeats) {
            return seatStatus[seatNumber];
        }
        return false;
    }

    string getFlightInfo() const {
        return flightInfo;
    }
};

class Ticket {
private:
    string passName;
    int seatNumber;
    string flightInfo;
    bool bookingStatus;
    Airplane* airplane;
public:
    Ticket(string name, int seat, Airplane* plane) : passName(name), seatNumber(seat), airplane(plane) {
        flightInfo = airplane->getFlightInfo();  // Get flight information from the Airplane object
        bookingStatus = false;  // Initialize booking status as false
    }

    // Method to book a ticket
    bool bookTicket() {
        if (airplane->bookSeat(seatNumber)) {
            bookingStatus = true;
            return true;  // Booking successful
        }
        return false;  // Booking failed
    }

    // Method to cancel a ticket
    bool cancelTicket() {
        if (airplane->cancelSeat(seatNumber)) {
            bookingStatus = false;
            return true;  // Cancellation successful
        }
        return false;  // Cancellation failed
    }

    // Method to display ticket information
    void displayTicketInfo() const {
        cout << "Passenger Name: " << passName << endl;
        cout << "Seat Number: " << seatNumber << endl;
        cout << "Flight Information: " << flightInfo << endl;
        cout << "Booking Status: " << (bookingStatus ? "Booked" : "Not Booked") << endl;
    }
};

class InputConfig {
public:
    // Method to read configuration from a file
    static void readConfig(const string& fileName, int& seats, string& flightInfo, double& price) {
        ifstream configFile(fileName);
        if (configFile.is_open()) {
            configFile >> seats;
            configFile.ignore();  // Ignore newline character after the number of seats
            getline(configFile, flightInfo);
            configFile >> price;
            configFile.close();
        } else {
            cerr << "Unable to open the configuration file!" << endl;
        }
    }
};

int main() {
    Airplane airplane(10, "Flight ABC123 from City A to City B");

    // Create a Ticket object for a passenger
    Ticket ticket("John Doe", 5, &airplane);

    // Book the ticket
    if (ticket.bookTicket()) {
        cout << "Ticket booked successfully!" << endl;
    } else {
        cout << "Failed to book the ticket." << endl;
    }

    // Display ticket information
    ticket.displayTicketInfo();

    // Cancel the ticket
    if (ticket.cancelTicket()) {
        cout << "Ticket canceled successfully!" << endl;
    } else {
        cout << "Failed to cancel the ticket." << endl;
    }

    // Display ticket information again
    ticket.displayTicketInfo();

    return 0;
}
