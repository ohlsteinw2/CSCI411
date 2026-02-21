/*
 * server.cpp
 * CSCI 411 - Cooperating Processes Lab
 *
 * This is the central process.
 * It receives temps from 4 clients,
 * calculates new central temperature,
 * and sends it back until everything stabilizes.
 *
 * Compile:
 *   g++ -std=c++17 server.cpp -o server -lrt
 */

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

using namespace std;

static const char* SERVER_QUEUE_NAME = "/ohlsteinw_server";
static const char* CLIENT_QUEUE_PREFIX = "/ohlsteinw_client";

#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10

enum MsgType { INIT = 1, UPDATE = 2, TERMINATE = 3 };

// Same structure as client
struct Packet {
    int type;
    int clientId;
    long temp;
};

static void die(const char* msg) {
    perror(msg);
    exit(1);
}

static string clientQueueName(int clientId) {
    return string(CLIENT_QUEUE_PREFIX) + to_string(clientId);
}

static void printTemp(long milli) {
    long whole = milli / 1000;
    long frac  = labs(milli % 1000);
    cout << whole << "." << (frac < 100 ? (frac < 10 ? "00" : "0") : "") << frac;
}

int main() {

    pid_t pid = getpid();
    cout << pid << ": server started\n";

    // Remove old server queue if it exists
    mq_unlink(SERVER_QUEUE_NAME);

    mq_attr attr{};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = sizeof(Packet);
    attr.mq_curmsgs = 0;

    // Create server queue (clients send here)
    mqd_t qd_server = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    if (qd_server == (mqd_t)-1) die("Server mq_open");

    long temps[4] = {0};
    long prevTemps[4] = {0};
    bool gotInit[4] = {false};

    cout << "Waiting for 4 clients...\n";

    // Wait until all 4 clients send INIT
    int count = 0;
    while (count < 4) {
        Packet p{};
        mq_receive(qd_server, reinterpret_cast<char*>(&p), sizeof(p), nullptr);

        if (p.type == INIT && !gotInit[p.clientId]) {
            temps[p.clientId] = p.temp;
            prevTemps[p.clientId] = p.temp;
            gotInit[p.clientId] = true;
            count++;

            cout << pid << ": received INIT from client "
                 << p.clientId << " temp=";
            printTemp(p.temp);
            cout << endl;
        }
    }

    // Open client queues for sending updates
    mqd_t qd_clients[4];
    for (int i = 0; i < 4; i++) {
        qd_clients[i] = mq_open(clientQueueName(i).c_str(), O_WRONLY);
        if (qd_clients[i] == (mqd_t)-1) die("Open client queue");
    }

    long central = 0;

    while (true) {

        // Compute central temperature
        // Formula:
        // (2*central + sum of external temps) / 6
        long sum = temps[0] + temps[1] + temps[2] + temps[3];
        central = (2 * central + sum) / 6;

        cout << pid << ": central temp=";
        printTemp(central);
        cout << endl;

        // Send central temp to all clients
        for (int i = 0; i < 4; i++) {
            Packet out{};
            out.type = UPDATE;
            out.clientId = i;
            out.temp = central;

            mq_send(qd_clients[i],
                    reinterpret_cast<const char*>(&out),
                    sizeof(out), 0);
        }

        // Receive updated temps
        long newTemps[4];
        int updateCount = 0;

        while (updateCount < 4) {
            Packet in{};
            mq_receive(qd_server,
                       reinterpret_cast<char*>(&in),
                       sizeof(in), nullptr);

            if (in.type == UPDATE) {
                newTemps[in.clientId] = in.temp;
                updateCount++;
            }
        }

        // Check if stable (exact match from last round)
        bool stable = true;
        for (int i = 0; i < 4; i++) {
            if (newTemps[i] != prevTemps[i]) {
                stable = false;
            }
        }

        // Update arrays
        for (int i = 0; i < 4; i++) {
            prevTemps[i] = temps[i];
            temps[i] = newTemps[i];
        }

        if (stable) {
            cout << pid << ": system is stable\n";

            // Tell clients to terminate
            for (int i = 0; i < 4; i++) {
                Packet term{};
                term.type = TERMINATE;
                term.clientId = i;
                term.temp = central;

                mq_send(qd_clients[i],
                        reinterpret_cast<const char*>(&term),
                        sizeof(term), 0);
            }
            break;
        }
    }

    // Clean up server queue
    mq_close(qd_server);
    mq_unlink(SERVER_QUEUE_NAME);

    return 0;
}

