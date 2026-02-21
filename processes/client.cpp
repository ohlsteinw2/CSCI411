/*
 * client.cpp
 * CSCI 411 - Cooperating Processes Lab
 *
 * This client represents one of the 4 external temperature processes.
 * It sends its temperature to the server and keeps updating until
 * the server says the system is stable.
 *

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

using namespace std;

// Names must start with / for POSIX message queues
static const char* SERVER_QUEUE_NAME = "/ohlsteinw_server";
static const char* CLIENT_QUEUE_PREFIX = "/ohlsteinw_client";

#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10

// Different types of messages we send
enum MsgType { INIT = 1, UPDATE = 2, TERMINATE = 3 };

// This is the structure both server and client agree on
// We send this back and forth through the message queue
struct Packet {
    int type;       // what kind of message
    int clientId;   // which client is sending
    long temp;      // temperature (stored as milli-degrees)
};

static void die(const char* msg) {
    perror(msg);
    exit(1);
}

// Helper to build client queue name like /ohlsteinw_client0
static string clientQueueName(int clientId) {
    return string(CLIENT_QUEUE_PREFIX) + to_string(clientId);
}

// Just prints temperature nicely (converts from milli-deg)
static void printTemp(long milli) {
    long whole = milli / 1000;
    long frac  = labs(milli % 1000);
    cout << whole << "." << (frac < 100 ? (frac < 10 ? "00" : "0") : "") << frac;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Usage: ./client <id 0..3>\n";
        return 1;
    }

    int clientId = atoi(argv[1]);
    if (clientId < 0 || clientId > 3) {
        cerr << "Client id must be 0,1,2,3\n";
        return 1;
    }

    // Initial temperatures given in the assignment
    // Stored as milli-degrees to avoid floating point issues
    const long initialTemps[4] = {100000, 22000, 50000, 40000};
    long myTemp = initialTemps[clientId];

    pid_t pid = getpid();
    string myQueue = clientQueueName(clientId);

    // Remove old queue in case program crashed before
    mq_unlink(myQueue.c_str());

    // Setup queue attributes
    mq_attr attr{};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = sizeof(Packet);
    attr.mq_curmsgs = 0;

    // Create client queue (this is where server sends replies)
    mqd_t qd_client = mq_open(myQueue.c_str(), O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    if (qd_client == (mqd_t)-1) die("Client mq_open");

    // Open server queue so we can send to it
    mqd_t qd_server = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (qd_server == (mqd_t)-1) {
        cerr << "Start server first.\n";
        return 1;
    }

    // Send initial temperature to server
    Packet initMsg{};
    initMsg.type = INIT;
    initMsg.clientId = clientId;
    initMsg.temp = myTemp;

    cout << pid << ": sending INIT temp=";
    printTemp(myTemp);
    cout << endl;

    mq_send(qd_server, reinterpret_cast<const char*>(&initMsg), sizeof(initMsg), 0);

    // Main loop: wait for central temp from server
    while (true) {

        Packet incoming{};
        mq_receive(qd_client, reinterpret_cast<char*>(&incoming), sizeof(incoming), nullptr);

        if (incoming.type == TERMINATE) {
            cout << pid << ": system stable, final temp=";
            printTemp(incoming.temp);
            cout << endl;
            break;
        }

        // Compute new external temperature
        // Formula from assignment:
        // new external = (3*myTemp + 2*central) / 5
        long centralTemp = incoming.temp;
        myTemp = (3 * myTemp + 2 * centralTemp) / 5;

        cout << pid << ": received central=";
        printTemp(centralTemp);
        cout << " sending updated temp=";
        printTemp(myTemp);
        cout << endl;

        Packet updateMsg{};
        updateMsg.type = UPDATE;
        updateMsg.clientId = clientId;
        updateMsg.temp = myTemp;

        mq_send(qd_server, reinterpret_cast<const char*>(&updateMsg), sizeof(updateMsg), 0);
    }

    // Clean up queue
    mq_close(qd_client);
    mq_unlink(myQueue.c_str());

    return 0;
}

