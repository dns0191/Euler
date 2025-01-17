#include "Variable.h"

void update_reactor_state(double time, double* data) {
    double PRE_INSERT_RHO = INSERT_RHO;

    // Check if time is within the range of HISTORY
    if (HISTORY.empty()) {
        throw std::runtime_error("HISTORY map is empty. Cannot proceed with update.");
    }
    if (time < HISTORY.begin()->first || time > HISTORY.rbegin()->first) {
        throw std::runtime_error("Time value is outside the range of HISTORY map.");
    }

    // Dynamically calculate rod positions
    std::tuple<double, double, double> POSITION = getInterpolatedTuple(HISTORY, time);
    FR_POSITION = std::get<0>(POSITION);
    CR_POSITION = std::get<1>(POSITION);

    // Dynamically calculate `INSERT_RHO`
    double frWorth = getInterpolatedValue(CR, FR_POSITION);
    double crWorth = getInterpolatedValue(FR, CR_POSITION);
    INSERT_RHO = (frWorth + crWorth) * 1e-5;

    // Update RHO and power
    double deltaRHO = INSERT_RHO - PRE_INSERT_RHO;
    RHO += deltaRHO;

    double k = 1 / (1 - RHO);
    double PGEN = PN_LIFE / k;
    double DELAY = 0.0, PROMPT = 0.0, deltaPOWER = 0.0;

    for (const auto& pair : PRECURSOR) {
        DELAY += pair.second * LAMBDA[pair.first];
    }
    PROMPT = (RHO - BETA_EFF) * POWER / PGEN;
    deltaPOWER = PROMPT + DELAY;

    // Update precursor groups
    std::map<int, double> deltaPRECURSOR;
    for (const auto& pair : PRECURSOR) {
        deltaPRECURSOR[pair.first] = pair.second * -LAMBDA[pair.first] + BETA[pair.first] * POWER / PGEN;
    }

    // Apply changes
    POWER += deltaPOWER * T_INTERVAL;
    for (auto& pair : PRECURSOR) {
        pair.second += deltaPRECURSOR[pair.first] * T_INTERVAL;
    }

    // Update data array
    data[0] = POWER;
    data[2] = FR_POSITION;
    data[3] = CR_POSITION;
    data[4] = RHO * 1e5;
    data[5] = INSERT_RHO * 1e5;

    // Dynamically fetch historical power
    auto it = HISTORY.lower_bound(time);
    if (it != HISTORY.begin() && (it == HISTORY.end() || it->first != time)) {
        --it;
    }
    if (it != HISTORY.end()) {
        data[1] = std::get<2>(it->second);
    }
}
