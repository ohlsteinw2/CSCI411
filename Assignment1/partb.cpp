#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

string readFirstLine(const string& filePath) {
    ifstream inFS(filePath);
    string line = "N/A";

    if (!inFS.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return "N/A";
    }

    getline(inFS, line);
    return line;
}
string findValue(const string& filePath, const string& key) {
    ifstream inFS(filePath);
    string line;

    if (!inFS.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return "N/A";
    }

    while (getline(inFS, line)) {
        if (line.find(key) == 0) {
            return line.substr((int)key.length());
        }
    }

    return "N/A";
}

void convert(double seconds, int& hours, int& minutes, int& secs) {
    int total = (int)floor(seconds);
    hours = total / 3600;
    total %= 3600;
    minutes = total / 60;
    secs = total % 60;
}

int cpuCount() {
    ifstream inFS("/proc/cpuinfo");
    string line;
    int count = 0;

    if (!inFS.is_open()) {
        cerr << "Error opening /proc/cpuinfo" << endl;
        return 0;
    }

    while (getline(inFS, line)) {
        if (line.find("processor") == 0) {
            count++;
        }
    }

    return count;
}

void readCPU(string& processor, string& vendor, string& model, string& modelName) {
    processor = "N/A";
    vendor = "N/A";
    model = "N/A";
    modelName = "N/A";

    ifstream inFS("/proc/cpuinfo");
    string line;

    if (!inFS.is_open()) {
        cerr << "Error opening /proc/cpuinfo" << endl;
        return;
    }

    while (getline(inFS, line)) {
        if (line.empty()) break; 

        if (line.find("processor") == 0) {
            processor = line.substr((int)line.find(":") + 2);
        }
        else if (line.find("vendor_id") == 0) {
            vendor = line.substr((int)line.find(":") + 2);
        }
        else if (line.find("model name") == 0) {
            modelName = line.substr((int)line.find(":") + 2);
        }
        else if (line.find("model") == 0) {
            model = line.substr((int)line.find(":") + 2);
        }
    }
}

int main() {
    string host = readFirstLine("/proc/sys/kernel/hostname");
    int cpus = cpuCount();
    string processor, vendor, model, modelName;
    readCPU(processor, vendor, model, modelName);
    string kernel = readFirstLine("/proc/version");
    double upSeconds = 0.0, idleSeconds = 0.0;
    ifstream upFS("/proc/uptime");
    if (upFS.is_open()) {
        upFS >> upSeconds >> idleSeconds;
    }

    int uptimeh, uptimem, uptimes, idleh, idlem, idles;
    convert(upSeconds, uptimeh, uptimem, uptimes);
    convert(idleSeconds, idleh, idlem, idles);

    // F: Memory
    string memTotal = findValue("/proc/meminfo", "MemTotal:");
    string memFree  = findValue("/proc/meminfo", "MemFree:");

    // OUTPUT
    cout << "A: Host name : " << host << "\n\n";

    cout << "B: Number of processing units: " << cpus << "\n\n";

    cout << "C: CPU(s) Type and model :\n";
    cout << "processor : " << processor << "\n";
    cout << "vendor_id : " << vendor << "\n";
    cout << "model : " << model << "\n";
    cout << "model name : " << modelName << "\n\n";

    cout << "D: Linux Kernel Version: " << kernel << "\n\n";

    cout << "E: System time:\n";
    cout << "Time since last re-boot: " << (int)floor(upSeconds)
         << " seconds which is equivalent to "
         << uptimeh << " hours " << uptimem << " minutes " << uptimes << " seconds.\n";

    cout << "Time in idle is " << (int)floor(idleSeconds)
         << " seconds which is equivalent to "
         << idleh << " hours " << idlem << " minutes " << idles << " seconds.\n\n";

    cout << "F: Memory information\n";
    cout << "MemTotal:" << memTotal << "\n";
    cout << "MemFree:" << memFree << "\n";

    return 0;
}
