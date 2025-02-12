#include <iostream>
#include <pthread.h>
#include <queue>
#include <fstream>
#include <unistd.h> 
#include <cstdlib>  
#include <ctime>
#include <vector>
#define MAX_BUFFER_SIZE 5 

using namespace std;
int totalTickets, numAgents;

struct customer {
    string name;
    int tickets;
};
typedef struct customer Customer;

// the size of this queue should be limited to MAX_BUFFER_SIZE
queue<Customer> customerRequests;


void bookTicket() {
    Customer currentCustomer = customerRequests.front();
    customerRequests.pop();

    int ticketsRequested = currentCustomer.tickets;
    string name = currentCustomer.name;

    if(totalTickets >= ticketsRequested) {
        totalTickets -= ticketsRequested;
        cout << "Customer " << name << " given " << ticketsRequested << " tickets\n";
        
    } 
    else if(totalTickets <= 0) {
        cout << "Customer " << name << " given 0 tickets\n";
    }
    else {
        int ticketsRemaining = ticketsRequested - totalTickets;
        cout << "Customer " << name << " given " << totalTickets << " tickets\n";
        totalTickets = 0;
    }
}

int main() {
    ifstream file("sample_input.txt");
    if (!file) {
        cerr << "Error: Could not open input.txt\n";
        exit(1);
    }

    string name;
    
    file >> totalTickets;
    
    int tickets;

    while (file >> name) {
        file >> tickets;
        Customer customer = {name, tickets};
        // print the number of tickets requested
        cout << "Customer " << name << " requested " << tickets << " tickets\n";

        // in this, customerRequests queue will have only one ticket request at a time
        // feel free to modify this in the parallel implementation
        // customerRequests can have up to <MAX_BUFFER_SIZE> requests at once
        customerRequests.push(customer);
        bookTicket();
    }

    file.close();
    cout << "Remaining tickets: " << totalTickets;
    return 0;
}

