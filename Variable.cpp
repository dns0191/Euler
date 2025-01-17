// Variable.cpp
#include "Variable.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <tuple>
#include <string>
#include <iomanip>
#include <cmath>

double RHO = 0.0;
double INSERT_RHO = 0.0;
double POWER = 0.0;
double FR_POSITION = 0.0;
double CR_POSITION = 0.0;
double PN_LIFE = 0.0;
double T_INTERVAL = 0.0;
double BETA_EFF = 0.0;
double END_TIME = 0.0;

std::map<int, double> BETA;
std::map<int, double> LAMBDA;
std::map<int, double> PRECURSOR;

std::map<double, double> CR;
std::map<double, double> FR;
std::map<double, std::tuple<double, double, double>> HISTORY;

std::map<double, double> INTERPOLATED_CR;
std::map<double, double> INTERPOLATED_FR;
std::map<double, std::tuple<double, double, double>> INTERPOLATED_HISTORY;

static std::map<double, double> linearInterpolate(const std::map<double, double>& data, double interval) {

    std::map<double, double> interpolatedData;

    if (data.size() < 2 || interval <= 0.0) {
        throw std::invalid_argument("Insufficient data or invalid interval for interpolation.");
    }

    auto it1 = data.begin();
    auto it2 = std::next(it1);

    while (it2 != data.end()) {
        double startKey = it1->first;
        double endKey = it2->first;
        double startValue = it1->second;
        double endValue = it2->second;

        for (double key = startKey; key <= endKey; key += interval) {
            if (key > endKey) break;

            double value = startValue + (key - startKey) / (endKey - startKey) * (endValue - startValue);
            interpolatedData[key] = value;
        }
        interpolatedData[endKey] = endValue;

        it1++;
        it2++;
    }

    return interpolatedData;
}

// std::tuple<double, double, double> 보간
static std::map<double, std::tuple<double, double, double>> linearInterpolate(const std::map<double, std::tuple<double, double, double>>& data, double interval) {

    std::map<double, std::tuple<double, double, double>> interpolatedData;

    if (data.size() < 2 || interval <= 0.0) {
        throw std::invalid_argument("Insufficient data or invalid interval for interpolation.");
    }

    auto it1 = data.begin();
    auto it2 = std::next(it1);

    while (it2 != data.end()) {
        double startKey = it1->first;
        double endKey = it2->first;
        const auto& startValue = it1->second;
        const auto& endValue = it2->second;

        for (double key = startKey; key <= endKey; key += interval) {
            if (key > endKey) break;

            double value1 = std::get<0>(startValue) + (key - startKey) / (endKey - startKey) * (std::get<0>(endValue) - std::get<0>(startValue));
            double value2 = std::get<1>(startValue) + (key - startKey) / (endKey - startKey) * (std::get<1>(endValue) - std::get<1>(startValue));
            double value3 = std::get<2>(startValue) + (key - startKey) / (endKey - startKey) * (std::get<2>(endValue) - std::get<2>(startValue));
            interpolatedData[key] = std::make_tuple(value1, value2, value3);
        }
        interpolatedData[endKey] = endValue;

        it1++;
        it2++;
    }

    return interpolatedData;
}

double findWorth(const std::map<double, double>& data, double position) {
    auto lower = data.lower_bound(position);
    if (lower == data.end()) {
        return (--lower)->second;
    }
    if (lower == data.begin()) {
        return lower->second;
    }
    auto upper = lower--;
    if (std::abs(position - lower->first) < std::abs(position - upper->first)) {
        return lower->second;
    }
    return upper->second;
}

std::tuple<double, double> findRodPosition(double time) {
    auto lower = INTERPOLATED_HISTORY.lower_bound(time);
    if (lower == INTERPOLATED_HISTORY.end()) {
        return std::make_tuple(std::get<0>((--lower)->second), std::get<1>(lower->second));
    }
    if (lower == INTERPOLATED_HISTORY.begin()) {
        return std::make_tuple(std::get<0>(lower->second), std::get<1>(lower->second));
    }
    auto upper = lower--;
    if (std::abs(time - lower->first) < std::abs(time - upper->first)) {
        return std::make_tuple(std::get<0>(lower->second), std::get<1>(lower->second));
    }
    return std::make_tuple(std::get<0>(upper->second), std::get<1>(upper->second));
}

void loadSimulationData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "rho") {
            iss >> RHO;
        }
        else if (key == "power") {
            iss >> POWER;
        }
        else if (key == "i_rod(FR)") {
            iss >> FR_POSITION;
        }
        else if (key == "i_rod(CR)") {
            iss >> CR_POSITION;
        }
        else if (key == "pn_life") {
            iss >> PN_LIFE;
        }
        else if (key == "t_interval") {
            iss >> T_INTERVAL;
        }
        else if (key == "beta") {
            while (std::getline(file, line) && !line.empty() && line.find("LAMBDA") == std::string::npos) {
                std::istringstream tableLine(line);
                int index;
                double value;
                tableLine >> index >> value;
                BETA.insert({ index, value });
            }
        }
        else if (key == "lambda") {
            while (std::getline(file, line) && !line.empty() && line.find("CR_position") == std::string::npos) {
                std::istringstream tableLine(line);
                int index;
                double value;
                tableLine >> index >> value;
                LAMBDA.insert({ index, value });
            }
        }
        else if (key == "CR_position") {
            while (std::getline(file, line) && !line.empty() && line.find("FR_position") == std::string::npos) {
                std::istringstream tableLine(line);
                double position, worth;
                tableLine >> position >> worth;
                CR.insert({ position, worth });
            }
        }
        else if (key == "FR_position") {
            while (std::getline(file, line) && !line.empty()) {
                std::istringstream tableLine(line);
                double position, worth;
                tableLine >> position >> worth;
                FR.insert({ position, worth });
            }
        }
    }

    file.close();

    // BETA_EFF 계산
    BETA_EFF = 0.0;
    for (const auto& beta : BETA) {
        BETA_EFF += beta.second;
    }

    // 초기 PRECURSOR 계산
    for (int i = 1; i <= 6; i++) {
        PRECURSOR[i] = BETA[i] * POWER / (LAMBDA[i] * PN_LIFE);
    }

    // 제어봉가 보간
    INTERPOLATED_CR = linearInterpolate(CR, T_INTERVAL*0.01);
    INTERPOLATED_FR = linearInterpolate(FR, T_INTERVAL*0.01);

    // INSERT_RHO 계산
    double frWorth = findWorth(INTERPOLATED_FR, FR_POSITION);
    double crWorth = findWorth(INTERPOLATED_CR, CR_POSITION);
	INSERT_RHO = (frWorth + crWorth)*1e-5;
}

void loadHistoryData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        double time, rodFR, rodCR, power;
        iss >> time >> rodFR >> rodCR >> power;
        HISTORY.insert({ time,{rodFR,rodCR,power} });
    }

    file.close();

    // 운전 기록 보간
    INTERPOLATED_HISTORY = linearInterpolate(HISTORY, T_INTERVAL);

    // HISTORY에서 마지막 Time 값을 END_TIME으로 설정
    if (!HISTORY.empty()) {
        END_TIME = HISTORY.rbegin()->first;
    }
}

void calculateInsertRho(double time) {
    std::tuple<double, double> POSITION= findRodPosition(time);
    double frWorth = findWorth(INTERPOLATED_FR, std::get<0>(POSITION));
    double crWorth = findWorth(INTERPOLATED_CR, std::get<1>(POSITION));
    INSERT_RHO = (frWorth + crWorth)*1e-5;
}

void printSimulationData() {
    std::ofstream file("simulation_data.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: simulation_data.txt");
    }

    file << "RHO: " << RHO << "\n";
    file << "INSERT_RHO: " << INSERT_RHO << "\n";
    file << "Power: " << POWER << "\n";
    file << "I_ROD_FR: " << FR_POSITION << "\n";
    file << "I_ROD_CR: " << CR_POSITION << "\n";
    file << "PN_LIFE: " << PN_LIFE << "\n";
    file << "T_INTERVAL: " << T_INTERVAL << "\n";
    file << "BETA_EFF: " << BETA_EFF << "\n";
    file << "END_TIME: " << END_TIME << "\n";

    file << "BETA:\n";
    for (std::map<int, double>::iterator it = BETA.begin(); it != BETA.end(); ++it) {
        file << it->first << "\t" << it->second << "\n";
    }

    file << "LAMBDA:\n";
    for (std::map<int, double>::iterator it = LAMBDA.begin(); it != LAMBDA.end(); ++it) {
        file << it->first << "\t" << it->second << "\n";
    }

    file << "PRECURSOR:\n";
    for (std::map<int, double>::iterator it = PRECURSOR.begin(); it != PRECURSOR.end(); ++it) {
        file << it->first << "\t" << it->second << "\n";
    }

    file << "CR Data:\n";
    for (std::map<double, double>::iterator it = INTERPOLATED_CR.begin(); it != INTERPOLATED_CR.end(); ++it) {
        file << it->first << "\t" << it->second << "\n";
    }

    file << "FR Data:\n";
    for (std::map<double, double>::iterator it = INTERPOLATED_FR.begin(); it != INTERPOLATED_FR.end(); ++it) {
        file << it->first << "\t" << it->second << "\n";
    }

    file.close();
}

void printHistoryData() {
    std::ofstream file("history_data.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: history_data.txt");
    }

    file << std::fixed; // 소수점 고정 및 자릿수 설정
    file << "Time\trod(FR)\trod(CR)\tPower\n";

    for (const auto& entry : INTERPOLATED_HISTORY) {
        file << std::setprecision(3) << entry.first << "\t"  // 키 (Time)
            << std::setprecision(2) << std::get<0>(entry.second) << "\t"  // rod(FR)
            << std::setprecision(4) << std::get<1>(entry.second) << "\t"  // rod(CR)
            << std::scientific << std::get<2>(entry.second) << "\n"; // Power: 지수 포맷
        file << std::fixed;  // 다음 루프에서 고정 소수점 형식으로 돌아가기
    }

    file.close();
}
