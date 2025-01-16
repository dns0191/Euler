#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include "Variable.h"
#include "Function.h"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    double TIME = 0.0;
    double nextSaveTime = 0.0;
    double nextProgressTime = 10.0;
    double DATA[6] = { 0.0 };
    loadSimulationData("Input.inp");
    loadHistoryData("history.inp");

    std::ofstream outputFile("output.out");
    if (!outputFile.is_open()) {
        std::cerr << "Unable to open file for writing: output.out" << std::endl;
        return 1;
    }

    // 헤더 출력
    outputFile << std::setw(10) << "Time"
        << std::setw(15) << "Power"
        << std::setw(15) << "Hist_Power"
        << std::setw(15) << "FR_Position"
        << std::setw(15) << "CR_Position"
        << std::setw(15) << "RHO"
        << std::setw(15) << "InsertRho"
        << std::endl;
    std::cout << "Simulation started." << std::endl;

    while (TIME <= END_TIME) {
        update_reactor_state(TIME, DATA);

        // 데이터 저장 (1초 단위로)
        if (TIME >= nextSaveTime) {
            outputFile << std::setw(10) << std::fixed << int(TIME);
            outputFile << std::setw(15) << std::scientific << std::setprecision(5) << DATA[0]; // Power
            outputFile << std::setw(15) << std::scientific << std::setprecision(5) << DATA[1]; // Hist_Power
            for (int i = 2; i < 6; ++i) {
                outputFile << std::setw(15) << std::fixed << std::setprecision(4) << DATA[i];
            }
            outputFile << std::endl;
            nextSaveTime += 1.0;
        }

        // 진행 상황 출력 (10초 단위로)
        if (TIME >= nextProgressTime) {
            std::cout << "Progress: " << int(TIME) << " seconds" << std::endl;
            nextProgressTime += 10.0;
        }

        TIME += T_INTERVAL;
    }
    outputFile << std::setw(10) << std::fixed << std::setprecision(0) << TIME;
    outputFile << std::setw(15) << std::scientific << std::setprecision(5) << DATA[0]; // Power
    outputFile << std::setw(15) << std::scientific << std::setprecision(5) << DATA[1]; // Hist_Power
    for (int i = 2; i < 6; ++i) {
        outputFile << std::setw(15) << std::fixed << std::setprecision(4) << DATA[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    outputFile << "\nCPU Time: " << elapsed.count() << " seconds" << std::endl;

    outputFile.close();

	std::cout << "CPU Time: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}
