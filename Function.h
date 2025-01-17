#include "Variable.h"

void update_reactor_state(double time, double* data) {
    if (HISTORY.empty() || time < HISTORY.begin()->first || time > HISTORY.rbegin()->first) {
        throw std::runtime_error("Invalid time or empty HISTORY map.");
    }
    double PRE_INSERT_RHO = INSERT_RHO;
    auto POSITION = getInterpolatedTuple(HISTORY, time);
    FR_POSITION = std::get<0>(POSITION);
    CR_POSITION = std::get<1>(POSITION);

    double frWorth = findWorth(FR, FR_POSITION);
    double crWorth = findWorth(CR, CR_POSITION);
    INSERT_RHO = (frWorth + crWorth) * 1e-5;

    RHO += INSERT_RHO - PRE_INSERT_RHO;

    double k = 1 / (1 - RHO);
    double PGEN = PN_LIFE / k;
    double PROMPT = (RHO - BETA_EFF) * POWER / PGEN;
    double DELAY = 0.0;

    for (const auto& pair : PRECURSOR) {
        DELAY += pair.second * LAMBDA[pair.first];
    }

    POWER += (PROMPT + DELAY) * T_INTERVAL;

    for (auto& pair : PRECURSOR) {
        pair.second += (pair.second * -LAMBDA[pair.first] + BETA[pair.first] * POWER / PGEN) * T_INTERVAL;
    }

    data[0] = POWER;
    data[1] = std::get<2>(POSITION);
    data[2] = FR_POSITION;
    data[3] = CR_POSITION;
    data[4] = RHO * 1e5;
    data[5] = INSERT_RHO * 1e5;
}
