#include "Variable.h"
#include <tuple>
#include <map>

double k;
double PGEN;
double PROMPT;
double DELAY;
double PRE_INSERT_RHO;
double frWorth;
double crWorth;
std::tuple<double, double, double> POSITION;

void update_reactor_state(double time, double* data) {
    // RK4 coefficients
    auto f = [&](double t, double* state, double dt) -> double* {
        double localState[6] = { state[0], state[1], state[2], state[3], state[4], state[5] };

        PRE_INSERT_RHO = INSERT_RHO;
        POSITION = getInterpolatedTuple(HISTORY, t);
        FR_POSITION = std::get<0>(POSITION);
        CR_POSITION = std::get<1>(POSITION);

        frWorth = findWorth(FR, FR_POSITION);
        crWorth = findWorth(CR, CR_POSITION);
        INSERT_RHO = (frWorth + crWorth) * 1e-5;

        RHO += INSERT_RHO - PRE_INSERT_RHO;

        k = 1 / (1 - RHO);
        PGEN = PN_LIFE / k;
        PROMPT = (RHO - BETA_EFF) * state[0] / PGEN;
        DELAY = 0.0;

        for (auto& pair : PRECURSOR) {
            DELAY += pair.second * LAMBDA[pair.first];
        }

        for (auto& pair : PRECURSOR) {
            pair.second += (pair.second * -LAMBDA[pair.first] + BETA[pair.first] * state[0] / PGEN) * dt;
        }

        localState[0] += (PROMPT + DELAY) * dt;
        localState[1] = std::get<2>(POSITION);
        localState[2] = FR_POSITION;
        localState[3] = CR_POSITION;
        localState[4] = RHO * 1e5;
        localState[5] = INSERT_RHO * 1e5;

        return localState;
        };

    double k1[6] = { 0.0 }, k2[6] = { 0.0 }, k3[6] = { 0.0 }, k4[6] = { 0.0 };
    double intermediateState[6] = { 0.0 };

    // Compute k1
    auto k1State = f(time, data, 0.0);
    for (int i = 0; i < 6; ++i) k1[i] = k1State[i];

    // Compute k2
    for (int i = 0; i < 6; ++i) intermediateState[i] = data[i] + k1[i] * T_INTERVAL / 2;
    auto k2State = f(time + T_INTERVAL / 2, intermediateState, T_INTERVAL / 2);
    for (int i = 0; i < 6; ++i) k2[i] = k2State[i];

    // Compute k3
    for (int i = 0; i < 6; ++i) intermediateState[i] = data[i] + k2[i] * T_INTERVAL / 2;
    auto k3State = f(time + T_INTERVAL / 2, intermediateState, T_INTERVAL / 2);
    for (int i = 0; i < 6; ++i) k3[i] = k3State[i];

    // Compute k4
    for (int i = 0; i < 6; ++i) intermediateState[i] = data[i] + k3[i] * T_INTERVAL;
    auto k4State = f(time + T_INTERVAL, intermediateState, T_INTERVAL);
    for (int i = 0; i < 6; ++i) k4[i] = k4State[i];

    // Combine to compute final state
    for (int i = 0; i < 6; ++i) {
        data[i] += (T_INTERVAL / 6.0) * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
    }
}
