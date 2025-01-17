#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include "Variable.h"
#include "Function.h"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    double TIME = 0.0, nextSaveTime = 0.0, DATA[6] = { 0.0 };
    loadSimulationData("Input.inp");
    loadHistoryData("history.inp");

    std::ofstream outputFile("output.out");
    if (!outputFile.is_open()) {
        throw std::runtime_error("Unable to open file: output.out");
    }

    outputFile << "Time\tPower\tHist_Power\tFR_Position\tCR_Position\tRHO\tInsertRho\n";

    while (TIME <= END_TIME) {
        update_reactor_state(TIME, DATA);

        if (TIME >= nextSaveTime) {
            outputFile << TIME << '\t' << DATA[0] << '\t' << DATA[1] << '\t'
                << DATA[2] << '\t' << DATA[3] << '\t' << DATA[4] << '\t' << DATA[5] << '\n';
            nextSaveTime += 1.0;
        }

        TIME += T_INTERVAL;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Simulation completed in " << std::chrono::duration<double>(end - start).count() << " seconds.\n";
    outputFile.close();

    return 0;
}
