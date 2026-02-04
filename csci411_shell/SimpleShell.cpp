#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>

using namespace std;

extern char **environ;

static const char *HISTORY_FILE = "myshell_history.txt";
static volatile sig_atomic_t got_sigint = 0;

void sigint_handler(int sig) {
    got_sigint = 1;
}

void printHistory() {
    ifstream in(HISTORY_FILE);
    cout << "\n===== COMMAND HISTORY =====\n";
    string line;
    while (getline(in, line)) {
        cout << line << endl;
    }
    cout << "===== END HISTORY =====\n";
}

void help() {
    cout << "MyShell Help\n";
    cout << "------------\n";
    cout << "myprocess       - show shell process info\n";
    cout << "allprocesses    - show all processes\n";
    cout << "chgd <dir>      - change directory\n";
    cout << "clr             - clear screen\n";
    cout << "dir <dir>       - list directory\n";
    cout << "environ         - show environment variables\n";
    cout << "repeat <str>    - echo string (supports > file)\n";
    cout << "hiMom           - fork + pipe demo\n";
    cout << "quit            - exit shell\n";
}

void myprocess() {
    cout << "PID: " << getpid() << endl;
    cout << "Parent PID: " << getppid() << endl;
}

void allprocesses() {
    system("ps -ef");
}

void clr() {
    cout << "\033[2J\033[H";
}

void environ_cmd() {
    for (char **env = environ; *env; env++) {
        cout << *env << endl;
    }
}

void dir_cmd(const string &path) {
    string cmd = "ls -al ";
    if (path.empty())
        cmd += ".";
    else
        cmd += path;
    system(cmd.c_str());
}

void repeat_cmd(const string &line) {
    size_t pos = line.find(">");
    string msg, file;

    if (pos != string::npos) {
        msg = line.substr(0, pos);
        file = line.substr(pos + 1);

        // trim
        msg.erase(0, msg.find_first_not_of(" \t"));
        file.erase(0, file.find_first_not_of(" \t"));

        ofstream out(file.c_str());
        out << msg << endl;
        out.close();
    } else {
        cout << line << endl;
    }
}

void hiMom() {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        // Child
        close(fd[0]);
        const char *msg = "Hi Mom! (from child process)";
        write(fd[1], msg, strlen(msg));
        close(fd[1]);
        exit(0);
    } else {
        // Parent
        close(fd[1]);
        char buffer[128];
        int n = read(fd[0], buffer, sizeof(buffer) - 1);
        buffer[n] = '\0';
        cout << buffer << endl;
        close(fd[0]);
        waitpid(pid, NULL, 0);
    }
}

int main() {
    ofstream history(HISTORY_FILE, ios::app);

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    while (true) {
        if (got_sigint) {
            cout << "\nShell exited due to SIGINT\n";
            history.close();
            printHistory();
            return 0;
        }

        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        cout << "WilliamShell:" << cwd << "$ ";

        string line;
        getline(cin, line);

        history << line << endl;

        stringstream ss(line);
        string cmd;
        ss >> cmd;

        if (cmd == "quit") {
            history.close();
            printHistory();
            return 0;
        } 
        else if (cmd == "help") help();
        else if (cmd == "myprocess") myprocess();
        else if (cmd == "allprocesses") allprocesses();
        else if (cmd == "clr") clr();
        else if (cmd == "environ") environ_cmd();
        else if (cmd == "chgd") {
            string d;
            ss >> d;
            chdir(d.c_str());
        }
        else if (cmd == "dir") {
            string d;
            ss >> d;
            dir_cmd(d);
        }
        else if (cmd == "repeat") {
            string rest;
            getline(ss, rest);
            repeat_cmd(rest);
        }
        else if (cmd == "hiMom") {
            hiMom();
        }
        else {
            system(line.c_str());
        }
    }
}
