#ifndef OPERATOR_CONSOLE_H
#define OPERATOR_CONSOLE_H

#include <vector>
#include <string>
#include "Aircraft.h"
#include "Computer_System.h"
using namespace std;

class OperatorConsole {
private:
    vector<string> commands;

public:
    OperatorConsole();

    void storeCommands(const vector<string>& newCommands);

    void fetchPlaneData(Aircraft& plane);

    void sendMessage(CompSystem& compSystem);

    void printDetails(int id);

    void* operatorMain(void* arg);
};

#endif // OPERATOR_CONSOLE_H
