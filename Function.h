#ifndef FUNCTION_H
#define FUNCTION_H

#include "Variable.h"
#include <array>
#include <fstream>
//#include <iomanip>

// 디버그 파일 초기화 함수
inline void initialize_debug_files() {
    std::ofstream point_kinetic_debug_file("point_kinetic_debug.txt", std::ios::trunc);
    point_kinetic_debug_file.close();
    std::ofstream runge_kutta_debug_file("runge_kutta_debug.txt", std::ios::trunc);
    runge_kutta_debug_file.close();
}

inline std::array<double, 7> point_kinetic(double time, const double* data);
inline void update_reactor_state(double time, double* data);
inline void runge_kutta(double time, const double* data, double* delta);

inline void update_reactor_state(double time, double* data) {
    const double PRE_INSERT_RHO = INSERT_RHO;
    double DELTA[7] = { 0.0 };
    const std::tuple<double, double, double> position = getInterpolatedTuple(HISTORY, time);
    double STATE[7] = { 0.0 };
    FR_POSITION = std::get<0>(position);
    CR_POSITION = std::get<1>(position);
    INSERT_RHO = (findWorth(FR, FR_POSITION) + findWorth(CR, CR_POSITION)) * 1e-5;
    RHO += INSERT_RHO - PRE_INSERT_RHO;

	STATE[0] = POWER;
	for (const auto& pair : PRECURSOR) {
		STATE[pair.first] = pair.second;
	}
    runge_kutta(time, STATE, DELTA);

    for (auto& pair : PRECURSOR) {
        pair.second += DELTA[pair.first];
    }

    POWER += DELTA[0];

    data[0] = POWER;
    data[1] = std::get<2>(position);
    data[2] = FR_POSITION;
    data[3] = CR_POSITION;
    data[4] = RHO * 1e5;
    data[5] = INSERT_RHO * 1e5;
}

inline std::array<double, 7> point_kinetic(double time, const double* data) {
    std::array<double, 7> temp = { 0.0 };
    const double pre_insert_rho = INSERT_RHO;
    const double power = data[0];
    double precursor[6] = { 0.0 };
    const std::tuple<double, double, double> position = getInterpolatedTuple(HISTORY, time);
    const double fr_position = std::get<0>(position);
    const double cr_position = std::get<1>(position);
    const double insert_rho = (findWorth(FR, fr_position) + findWorth(CR, cr_position)) * 1e-5;

    const double delta_rho = insert_rho - pre_insert_rho;
    const double rho = RHO + delta_rho;
    const double k = 1 / (1 - rho);
    const double pgen = PN_LIFE / k;
    const double prompt = (rho - BETA_EFF) * power / pgen;
    double delay = 0.0;

    for (int i = 0; i < 6; i++) {
        precursor[i] = data[i + 1];
        delay += precursor[i] * LAMBDA[i + 1];
    }

    temp[0] = prompt + delay;

    for (auto& pair : PRECURSOR) {
        temp[pair.first] = pair.second * -LAMBDA[pair.first] + BETA[pair.first] * power / pgen;
    }

    // 디버깅 정보 파일에 기록
    /*std::ofstream debug_file("point_kinetic_debug.txt", std::ios_base::app);
    debug_file << std::fixed << std::setprecision(5);
    debug_file << "point_kinetic - time: " << time << ", Rho: " << rho * 1e5 << ", Prompt: " << std::scientific << prompt << ", Delay: " << delay << ", power: " << std::scientific << power <<", temp: ";
    for (const auto& val : temp) {
        debug_file << std::scientific << val << " ";
    }
    debug_file << ", precursor: ";
    for (const auto& val : precursor) {
        debug_file << std::scientific << val << " ";
    }
    debug_file << "\n";
    debug_file.close();*/

    return temp;
}

inline void runge_kutta(double time, const double* data, double* delta) {
    std::array<double, 7> temp = { 0.0 };

    const std::array<double, 7> k1 = point_kinetic(time, data);
    for (int i = 0; i < 7; i++) {
        temp[i] = data[i] + k1[i] * T_INTERVAL * 0.5;
    }

    const std::array<double, 7> k2 = point_kinetic(time + T_INTERVAL * 0.5, temp.data());
    for (int i = 0; i < 7; i++) {
        temp[i] = data[i] + k2[i] * T_INTERVAL * 0.5;
    }

    const std::array<double, 7> k3 = point_kinetic(time + T_INTERVAL * 0.5, temp.data());
    for (int i = 0; i < 7; i++) {
        temp[i] = data[i] + k3[i] * T_INTERVAL;
    }

    const std::array<double, 7> k4 = point_kinetic(time + T_INTERVAL, temp.data());
    for (int i = 0; i < 7; i++) {
        delta[i] += (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) * T_INTERVAL / 6;
    }

    // 디버깅 정보 파일에 기록
    /*std::ofstream debug_file("runge_kutta_debug.txt", std::ios_base::app);
    debug_file << std::fixed << std::setprecision(5);
    debug_file << "runge_kutta - time: " << time << ", DELTA: ";
    for (int i = 0; i < 7; i++) {
        debug_file << std::scientific << delta[i] << " ";
    }
    debug_file << ", STATE: ";
	for (int i = 0; i < 7; i++) {
		debug_file << std::scientific << data[i] << " ";
	}
    debug_file << "\n";
    debug_file.close();*/
}

#endif
