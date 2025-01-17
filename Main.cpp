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

    outputFile << std::left
        << std::setw(10) << "Time"
        << std::setw(15) << "Power"
        << std::setw(15) << "Hist_Power"
        << std::setw(15) << "FR_Position"
        << std::setw(15) << "CR_Position"
        << std::setw(15) << "RHO"
        << std::setw(15) << "InsertRho" << '\n';

    while (TIME <= END_TIME) {
        update_reactor_state(TIME, DATA);

        if (TIME >= nextSaveTime) {
            outputFile << std::fixed << std::setprecision(2)
                << std::setw(10) << TIME
                << std::scientific << std::setprecision(6)
                << std::setw(15) << DATA[0]
                << std::setw(15) << DATA[1]
                << std::fixed << std::setprecision(4)
                << std::setw(15) << DATA[2]
                << std::setw(15) << DATA[3]
                << std::setw(15) << DATA[4]
                << std::setw(15) << DATA[5] << '\n';
            nextSaveTime += 1.0;
        }

        TIME += T_INTERVAL;
    }

    // 마지막 데이터 출력
    if (TIME > nextSaveTime) {
        update_reactor_state(TIME, DATA);
        outputFile << std::fixed << std::setprecision(2)
            << std::setw(10) << TIME
            << std::scientific << std::setprecision(6)
            << std::setw(15) << DATA[0]
            << std::setw(15) << DATA[1]
            << std::fixed << std::setprecision(4)
            << std::setw(15) << DATA[2]
            << std::setw(15) << DATA[3]
            << std::setw(15) << DATA[4]
            << std::setw(15) << DATA[5] << '\n';
    }

    auto end = std::chrono::high_resolution_clock::now();

	outputFile << "CPU TIME: " << std::chrono::duration<double>(end - start).count() << " seconds.\n";

    outputFile.close();

    std::cout << "CPU TIME: " << std::chrono::duration<double>(end - start).count() << " seconds.\n";

    return 0;
}
