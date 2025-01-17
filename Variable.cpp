#include "Variable.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <tuple>
#include <map>

double RHO = 0.0, INSERT_RHO = 0.0, POWER = 0.0, FR_POSITION = 0.0, CR_POSITION = 0.0;
double PN_LIFE = 0.0, T_INTERVAL = 0.0, BETA_EFF = 0.0, END_TIME = 0.0;

std::map<int, double> BETA, LAMBDA, PRECURSOR;
std::map<double, double> CR, FR;
std::map<double, std::tuple<double, double, double>> HISTORY;

double findWorth(const std::map<double, double>& data, double position) {
    auto lower = data.lower_bound(position);
    if (lower == data.end()) return (--lower)->second;
    if (lower == data.begin()) return lower->second;

    auto upper = lower--;
    double denominator = upper->first - lower->first;
    if (denominator == 0.0) {
        throw std::runtime_error("Division by zero in interpolation.");
    }

    return lower->second + (position - lower->first) / denominator * (upper->second - lower->second);
}


std::tuple<double, double, double> getInterpolatedTuple(const std::map<double, std::tuple<double, double, double>>& data, double key) {
    auto lower = data.lower_bound(key);
    if (lower == data.end()) return (--lower)->second;
    if (lower == data.begin()) return lower->second;

    auto upper = lower--;
    double v1 = std::get<0>(lower->second) + (key - lower->first) / (upper->first - lower->first) * (std::get<0>(upper->second) - std::get<0>(lower->second));
    double v2 = std::get<1>(lower->second) + (key - lower->first) / (upper->first - lower->first) * (std::get<1>(upper->second) - std::get<1>(lower->second));
    double v3 = std::get<2>(lower->second) + (key - lower->first) / (upper->first - lower->first) * (std::get<2>(upper->second) - std::get<2>(lower->second));
    return { v1, v2, v3 };
}

void loadSimulationData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Unable to open file: " + filename);

    std::string line, key;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
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
            // Read beta values
            while (std::getline(file, line) && !line.empty()) {
                int group;
                double value;
                std::istringstream betaStream(line);
                betaStream >> group >> value;
                BETA[group] = value;
            }
        }
        else if (key == "lambda") {
            // Read lambda values
            while (std::getline(file, line) && !line.empty()) {
                int group;
                double value;
                std::istringstream lambdaStream(line);
                lambdaStream >> group >> value;
                LAMBDA[group] = value;
            }
        }
        else if (key == "CR_position") {
            // Read CR position data
            double position, worth;
            while (std::getline(file, line) && !line.empty()) {
                std::istringstream crStream(line);
                if (!(crStream >> position >> worth)) break;
                CR[position] = worth;
            }
        }
        else if (key == "FR_position") {
            // Read FR position data
            double position, worth;
            while (std::getline(file, line) && !line.empty()) {
                std::istringstream frStream(line);
                if (!(frStream >> position >> worth)) break;
                FR[position] = worth;
            }
        }
    }

    BETA_EFF = 0.0;
    for (const auto& beta : BETA) {
        BETA_EFF += beta.second;
    }

    // 초기 PRECURSOR 계산
    for (int i = 1; i <= 6; i++) {
        PRECURSOR[i] = BETA[i] * POWER / (LAMBDA[i] * PN_LIFE);
    }

	INSERT_RHO = (findWorth(FR, FR_POSITION) + findWorth(CR, CR_POSITION)) * 1e-5;

    // Ensure FR and CR maps are populated
    if (FR.empty()) throw std::runtime_error("FR data not loaded from Input.inp.");
    if (CR.empty()) throw std::runtime_error("CR data not loaded from Input.inp.");
}


void loadHistoryData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Unable to open file: " + filename);

    std::string line;
    double time, rodFR, rodCR, power;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        iss >> time >> rodFR >> rodCR >> power;
        HISTORY[time] = { rodFR, rodCR, power };
    }
    file.close();
    END_TIME = HISTORY.rbegin()->first;
}
