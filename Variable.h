// Variable.h
#ifndef SIMULATION_DATA_H
#define SIMULATION_DATA_H

#include <string>
#include <map>
#include <sstream>

extern double RHO;
extern double INSERT_RHO;
extern double POWER;
extern double FR_POSITION;
extern double CR_POSITION;
extern double PN_LIFE;
extern double T_INTERVAL;
extern double BETA_EFF;
extern double END_TIME;


extern std::map<int, double> BETA;
extern std::map<int, double> LAMBDA;
extern std::map<int, double> PRECURSOR;

extern std::map<double, double> CR;
extern std::map<double, double> FR;
extern std::map<double, std::tuple<double, double, double>> HISTORY;

double findWorth(const std::map<double, double>& data, double position);
void loadSimulationData(const std::string& filename);
void loadHistoryData(const std::string& filename);

std::tuple<double, double, double> getInterpolatedTuple(const std::map<double, std::tuple<double, double, double>>& data, double key);



#endif // SIMULATION_DATA_H
