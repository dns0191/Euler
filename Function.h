#ifndef FUNCTION_H
#define FUNCTION_H
#include "Variable.h"

double k;
double PGEN;
double PROMPT;
double DELAY;
double PRE_INSERT_RHO;
double frWorth;
double crWorth;
std::tuple<double, double, double> POSITION;

inline  void update_reactor_state(double time, double* data) {

    PRE_INSERT_RHO = INSERT_RHO;
    POSITION = getInterpolatedTuple(HISTORY, time);
    FR_POSITION = std::get<0>(POSITION);
    CR_POSITION = std::get<1>(POSITION);

    frWorth = findWorth(FR, FR_POSITION);
    crWorth = findWorth(CR, CR_POSITION);
    INSERT_RHO = (frWorth + crWorth) * 1e-5;

    RHO += INSERT_RHO - PRE_INSERT_RHO;

    k = 1 / (1 - RHO);
    PGEN = PN_LIFE / k;
    PROMPT = (RHO - BETA_EFF) * POWER / PGEN;
    DELAY = 0.0;

    for (auto& pair : PRECURSOR) {
        DELAY += pair.second * LAMBDA[pair.first];
    }

    for (auto& pair : PRECURSOR) {
        pair.second += (pair.second * -LAMBDA[pair.first] + BETA[pair.first] * POWER / PGEN) * T_INTERVAL;
    }
    
    POWER += (PROMPT + DELAY) * T_INTERVAL;

    data[0] = POWER;
    data[1] = std::get<2>(POSITION);
    data[2] = FR_POSITION;
    data[3] = CR_POSITION;
    data[4] = RHO * 1e5;
    data[5] = INSERT_RHO * 1e5;
}

#endif